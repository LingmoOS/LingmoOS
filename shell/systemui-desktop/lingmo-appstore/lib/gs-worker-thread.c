/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-worker-thread
 * @short_description: A worker thread which executes queued #GTasks until stopped
 *
 * #GsWorkerThread is a thread-safe wrapper around a #GTask queue and a single
 * worker thread which executes tasks on that queue.
 *
 * Tasks can be added to the queue using gs_worker_thread_queue(). The worker
 * thread (which is created when #GsWorkerThread is constructed) will execute
 * them in (priority, queue order) order. Each #GTaskThreadFunc is responsible
 * for calling `g_task_return_*()` on its #GTask to complete that task.
 *
 * The priority passed to gs_worker_thread_queue() will be used to adjust the
 * worker thread’s I/O priority (using `ioprio_set()`) when executing that task.
 *
 * It is intended that gs_worker_thread_queue() is an alternative to using
 * g_task_run_in_thread(). g_task_run_in_thread() queues tasks into a single
 * process-wide thread pool, so they are mixed in with other tasks, and it can
 * become hard to ensure the thread pool isn’t overwhelmed and that tasks are
 * executed in the right order.
 *
 * The worker thread will continue executing tasks until
 * gs_worker_thread_shutdown_async() is called. This must be called before the
 * final reference to the #GsWorkerThread is dropped.
 *
 * Since: 42
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>

#include "gs-ioprio.h"
#include "gs-worker-thread.h"

typedef enum {
	GS_WORKER_THREAD_STATE_RUNNING = 0,
	GS_WORKER_THREAD_STATE_SHUTTING_DOWN = 1,
	GS_WORKER_THREAD_STATE_SHUT_DOWN = 2,
} GsWorkerThreadState;

struct _GsWorkerThread
{
	GObject			 parent;

	gchar			*name;  /* (nullable) (owned) */

	GsWorkerThreadState	 worker_state;  /* (atomic) */
	GMainContext		*worker_context;  /* (owned); may be NULL before setup or after shutdown */
	GThread			*worker_thread;  /* (atomic); may be NULL before setup or after shutdown */

	GMutex			 queue_mutex;
	GQueue			 queue;
};

typedef enum {
	PROP_NAME = 1,
} GsWorkerThreadProperty;

static GParamSpec *props[PROP_NAME + 1] = { NULL, };

G_DEFINE_TYPE (GsWorkerThread, gs_worker_thread, G_TYPE_OBJECT)

typedef struct {
	GTaskThreadFunc work_func;
	GTask *task;  /* (owned) */
	gint priority;
} WorkData;

static void
work_data_free (WorkData *data)
{
	g_clear_object (&data->task);
	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (WorkData, work_data_free)

static void
gs_worker_thread_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
	GsWorkerThread *self = GS_WORKER_THREAD (object);

	switch ((GsWorkerThreadProperty) prop_id) {
	case PROP_NAME:
		g_value_set_string (value, self->name);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_worker_thread_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
	GsWorkerThread *self = GS_WORKER_THREAD (object);

	switch ((GsWorkerThreadProperty) prop_id) {
	case PROP_NAME:
		/* Construct only */
		g_assert (self->name == NULL);
		self->name = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_worker_thread_dispose (GObject *object)
{
	GsWorkerThread *self = GS_WORKER_THREAD (object);

	/* Should have stopped by now. */
	g_assert (self->worker_thread == NULL);

	g_clear_pointer (&self->name, g_free);
	g_clear_pointer (&self->worker_context, g_main_context_unref);

	g_mutex_lock (&self->queue_mutex);
	g_queue_clear_full (&self->queue, (GDestroyNotify) work_data_free);
	g_mutex_unlock (&self->queue_mutex);

	G_OBJECT_CLASS (gs_worker_thread_parent_class)->dispose (object);
}

static void
gs_worker_thread_finalize (GObject *object)
{
	GsWorkerThread *self = GS_WORKER_THREAD (object);

	g_mutex_clear (&self->queue_mutex);

	G_OBJECT_CLASS (gs_worker_thread_parent_class)->finalize (object);
}

static gpointer thread_cb (gpointer data);

static void
gs_worker_thread_constructed (GObject *object)
{
	GsWorkerThread *self = GS_WORKER_THREAD (object);

	G_OBJECT_CLASS (gs_worker_thread_parent_class)->constructed (object);

	/* Start up a worker thread and its #GMainContext. The worker will run
	 * and process events on @worker_context until @worker_state changes
	 * from %GS_WORKER_THREAD_STATE_RUNNING. */
	self->worker_state = GS_WORKER_THREAD_STATE_RUNNING;
	self->worker_context = g_main_context_new ();
	self->worker_thread = g_thread_new (self->name, thread_cb, self);
}

static void
gs_worker_thread_class_init (GsWorkerThreadClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->constructed = gs_worker_thread_constructed;
	object_class->get_property = gs_worker_thread_get_property;
	object_class->set_property = gs_worker_thread_set_property;
	object_class->dispose = gs_worker_thread_dispose;
	object_class->finalize = gs_worker_thread_finalize;

	/**
	 * GsWorkerThread:name: (not nullable):
	 *
	 * Name for the worker thread to use in debug output. This must be set.
	 *
	 * Since: 42
	 */
	props[PROP_NAME] =
		g_param_spec_string ("name",
				     "Name",
				     "Name for the worker thread to use in debug output.",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (props), props);
}

static void
gs_worker_thread_run_queue (GsWorkerThread *self)
{
	g_mutex_lock (&self->queue_mutex);
	while (!g_queue_is_empty (&self->queue)) {
		g_autoptr(WorkData) data = g_queue_pop_head (&self->queue);
		GTask *task;
		gpointer source_object;
		gpointer task_data;
		GCancellable *cancellable;

		/* thus the other threads can queue more work */
		g_mutex_unlock (&self->queue_mutex);

		task = data->task;
		source_object = g_task_get_source_object (task);
		task_data = g_task_get_task_data (task);
		cancellable = g_task_get_cancellable (task);

		/* Set the I/O priority of the thread to match the priority of the task. */
		gs_ioprio_set (data->priority);

		data->work_func (task, source_object, task_data, cancellable);

		g_mutex_lock (&self->queue_mutex);
	}
	g_mutex_unlock (&self->queue_mutex);
}

static gpointer
thread_cb (gpointer data)
{
	GsWorkerThread *self = GS_WORKER_THREAD (data);
	g_autoptr(GMainContextPusher) pusher = g_main_context_pusher_new (self->worker_context);

	while (g_atomic_int_get (&self->worker_state) != GS_WORKER_THREAD_STATE_SHUT_DOWN) {
		g_main_context_iteration (self->worker_context, TRUE);
		gs_worker_thread_run_queue (self);
	}

	return NULL;
}

static void
gs_worker_thread_init (GsWorkerThread *self)
{
	g_mutex_init (&self->queue_mutex);
	g_queue_init (&self->queue);
}

/**
 * gs_worker_thread_new:
 * @name: (not nullable): name for the worker thread
 *
 * Create and start a new #GsWorkerThread.
 *
 * @name will be used to set the thread name and in debug output.
 *
 * Returns: (transfer full): a new #GsWorkerThread
 * Since: 42
 */
GsWorkerThread *
gs_worker_thread_new (const gchar *name)
{
	g_return_val_if_fail (name != NULL, NULL);

	return g_object_new (GS_TYPE_WORKER_THREAD,
			     "name", name,
			     NULL);
}

static gint
gs_worker_thread_cmp (gconstpointer a,
		      gconstpointer b,
		      gpointer user_data)
{
	const WorkData *dta = a;
	const WorkData *dtb = b;
	return dta->priority - dtb->priority;
}

/**
 * gs_worker_thread_queue:
 * @self: a #GsWorkerThread
 * @priority: (default G_PRIORITY_DEFAULT): priority to queue the task at,
 *   typically #G_PRIORITY_DEFAULT
 * @work_func: (not nullable): function to run the task
 * @task: (transfer full) (not nullable): the #GTask containing context data to
 *   pass to @work_func
 *
 * Queue @task to be run in the worker thread at the given @priority.
 *
 * This function takes ownership of @task.
 *
 * @priority sets the order of the task in the queue, and also affects the I/O
 * priority of the worker thread when the task is executed — high priorities
 * result in a high I/O priority, low priorities result in an idle I/O priority,
 * as per `ioprio_set()`.
 *
 * When the task is run, @work_func will be executed and passed @task and the
 * source object, task data and cancellable set on @task.
 *
 * @work_func is responsible for calling `g_task_return_*()` on @task once the
 * task is complete.
 *
 * If a task is cancelled using its #GCancellable after it’s queued to the
 * #GsWorkerThread, @work_func will still be executed. @work_func is responsible
 * for checking whether the #GCancellable has been cancelled.
 *
 * It is an error to call this function after gs_worker_thread_shutdown_async()
 * has called.
 *
 * Since: 42
 */
void
gs_worker_thread_queue (GsWorkerThread  *self,
                        gint             priority,
                        GTaskThreadFunc  work_func,
                        GTask           *task)
{
	g_autoptr(WorkData) data = NULL;
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_WORKER_THREAD (self));
	g_return_if_fail (work_func != NULL);
	g_return_if_fail (G_IS_TASK (task));

	g_assert (g_atomic_int_get (&self->worker_state) == GS_WORKER_THREAD_STATE_RUNNING ||
		  g_task_get_source_tag (task) == gs_worker_thread_shutdown_async);

	data = g_new0 (WorkData, 1);
	data->work_func = work_func;
	data->task = g_steal_pointer (&task);
	data->priority = priority;

	locker = g_mutex_locker_new (&self->queue_mutex);
	g_queue_insert_sorted (&self->queue, g_steal_pointer (&data), gs_worker_thread_cmp, NULL);
	g_main_context_wakeup (self->worker_context);
}

/**
 * gs_worker_thread_is_in_worker_context:
 * @self: a #GsWorkerThread
 *
 * Returns whether the calling thread is the worker thread.
 *
 * This is intended to be used as a precondition check to ensure that worker
 * code is not accidentally run from the wrong thread.
 *
 * |[
 * static void
 * do_work (MyPlugin *self)
 * {
 *   g_assert (gs_worker_thread_is_in_worker_context (self->worker_thread));
 *
 *   // do some work
 * }
 * ]|
 *
 * Returns: %TRUE if running in the worker context, %FALSE otherwise
 * Since: 42
 */
gboolean
gs_worker_thread_is_in_worker_context (GsWorkerThread *self)
{
	return g_main_context_is_owner (self->worker_context);
}

static void shutdown_cb (GTask        *task,
                         gpointer      source_object,
                         gpointer      task_data,
                         GCancellable *cancellable);

/**
 * gs_worker_thread_shutdown_async:
 * @self: a #GsWorkerThread
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: callback for once the asynchronous operation is complete
 * @user_data: data to pass to @callback
 *
 * Shut down the worker thread.
 *
 * The thread will finish processing whatever task it’s currently processing
 * (if any), will return %G_IO_ERROR_CANCELLED for all remaining queued
 * tasks, and will then join the main process.
 *
 * This is a no-op if called subsequently.
 *
 * Since: 42
 */
void
gs_worker_thread_shutdown_async (GsWorkerThread      *self,
                                 GCancellable        *cancellable,
                                 GAsyncReadyCallback  callback,
                                 gpointer             user_data)
{
	g_autoptr(GTask) task = NULL;

	g_return_if_fail (GS_IS_WORKER_THREAD (self));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_worker_thread_shutdown_async);

	/* Already called? */
	if (g_atomic_int_get (&self->worker_state) != GS_WORKER_THREAD_STATE_RUNNING) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* Signal the worker thread to stop processing tasks. */
	g_atomic_int_set (&self->worker_state, GS_WORKER_THREAD_STATE_SHUTTING_DOWN);
	gs_worker_thread_queue (self, G_MAXINT  /* lowest priority */,
				shutdown_cb, g_steal_pointer (&task));
}

static void
shutdown_cb (GTask        *task,
             gpointer      source_object,
             gpointer      task_data,
             GCancellable *cancellable)
{
	GsWorkerThread *self = GS_WORKER_THREAD (source_object);
	gboolean updated_state;

	updated_state = g_atomic_int_compare_and_exchange (&self->worker_state,
							   GS_WORKER_THREAD_STATE_SHUTTING_DOWN,
							   GS_WORKER_THREAD_STATE_SHUT_DOWN);
	g_assert (updated_state);

	/* Tidy up. We can’t join the thread here as this function is executing
	 * within the thread and that would deadlock. */
	g_clear_pointer (&self->worker_context, g_main_context_unref);

	g_task_return_boolean (task, TRUE);
}

/**
 * gs_worker_thread_shutdown_finish:
 * @self: a #GsWorkerThread
 * @result: a #GAsyncResult
 * @error: return location for a #GError, or %NULL
 *
 * Finish an asynchronous shutdown operation started with
 * gs_worker_thread_shutdown_async();
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 42
 */
gboolean
gs_worker_thread_shutdown_finish (GsWorkerThread  *self,
                                  GAsyncResult    *result,
                                  GError         **error)
{
	gboolean success;

	g_return_val_if_fail (GS_IS_WORKER_THREAD (self), FALSE);
	g_return_val_if_fail (g_async_result_is_tagged (result, gs_worker_thread_shutdown_async), FALSE);
	g_return_val_if_fail (g_task_is_valid (result, self), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	success = g_task_propagate_boolean (G_TASK (result), error);

	if (success)
		g_thread_join (g_steal_pointer (&self->worker_thread));

	return success;
}
