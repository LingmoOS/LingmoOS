/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: tianshaoshuai <tianshaoshuai@kylinos.cn>
 *
 */


/*于2.5.1.0版本舍弃，专用python实现可视化工具*/

#include "../api/libkysettings.h"
#include "../api/ksettingsschema.h"

#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <gtk/gtk.h>

static GtkWidget *window = NULL;
static GtkWidget *left_scrolled_window = NULL;
static GtkWidget *right_scrolled_window = NULL;

// 左侧配置树
static GtkWidget *tree_folders = NULL;

// 左侧树选中节点对应的键值表
static GtkWidget *table_configure = NULL;

// tree_folders节点路径: 路径组成的schema id的配置数据
static GHashTable *configure_models = NULL;

enum ConfigureAttr
{
    COLUMN_NAME,
    COLUMN_TYPE,
    COLUMN_VALUE,
    COLUMN_SUMMARY,
    COLUMN_COUNT
};

static const char *title_array[] = {"名称\n",
                                    "路径\n",
                                    "摘要\n",
                                    "描述\n",
                                    "类型\n",
                                    "最小值\n",
                                    "最大值\n",
                                    "默认值\n",
                                    "当前值\n"};

void on_entry_button_clicked(GtkButton *button, gpointer user_data)
{
    char **list = g_strsplit(gtk_widget_get_name(GTK_WIDGET(button)), "|", -1);

    GtkTextBuffer *buffer = user_data;
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    KSettingsSchema *schema = g_hash_table_lookup(configure_models, list[0]);
    GHashTable *schema_key = g_hash_table_lookup(schema->values, list[1]);

    int is_compliance = 0;
    const gchar *endptr = NULL;
    GError *error = NULL;
    char *type = g_hash_table_lookup(schema_key, "value_type");
    if (0 != strcmp(type, "enum"))
    {
        GVariant *variant = g_variant_parse(G_VARIANT_TYPE(type), text, NULL, &endptr, &error);
        if (NULL == variant)
        {
            g_error_free(error);
            GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                       GTK_DIALOG_MODAL,
                                                       GTK_MESSAGE_INFO,
                                                       GTK_BUTTONS_OK,
                                                       "格式错误");

            // 设置消息框的标题
            gtk_window_set_title(GTK_WINDOW(dialog), "消息");

            // 显示消息框并等待用户响应
            gtk_dialog_run(GTK_DIALOG(dialog));

            // 销毁消息框
            gtk_widget_destroy(GTK_WIDGET(dialog));
            return;
        }
    }

    is_compliance = kdk_conf2_schema_key_range_check(schema_key, text);

    if (0 == is_compliance)
    {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_INFO,
                                                   GTK_BUTTONS_OK,
                                                   "设置的值不在范围内");

        // 设置消息框的标题
        gtk_window_set_title(GTK_WINDOW(dialog), "消息");

        // 显示消息框并等待用户响应
        gtk_dialog_run(GTK_DIALOG(dialog));

        // 销毁消息框
        gtk_widget_destroy(GTK_WIDGET(dialog));
        return;
    }
    GDBusConnection *connection = NULL;
    GDBusProxy *proxy = NULL;
    GVariant *result = NULL;
    int response = FALSE;

    // 获取 DBus 会话总线
    connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (error != NULL)
    {
        g_error_free(error);
        goto out;
    }

    // 创建代理对象
    proxy = g_dbus_proxy_new_sync(connection,
                                  G_DBUS_PROXY_FLAGS_NONE,
                                  NULL,
                                  "com.lingmo.lingmosdk.conf2",
                                  "/com/lingmo/lingmosdk/conf2",
                                  "com.lingmo.lingmosdk.conf2",
                                  NULL,
                                  &error);
    if (error != NULL)
    {
        g_error_free(error);
        goto out;
    }

    uid_t uid = getuid(); // 获取当前进程的用户ID
    char user_name[128] = {0};
    struct passwd *pw = getpwuid(uid); // 获取用户ID对应的用户信息结构体
    if (NULL == pw)
    {
        strcpy(user_name, "root");
    }
    else
    {
        strcpy(user_name, pw->pw_name);
        endpwent();
    }

    GVariant *variant = g_variant_new("(sssss)", user_name, schema->id, schema->version, list[1], text);
    result = g_dbus_proxy_call_sync(proxy,
                                    "set",
                                    variant,
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    &error);
    if (error != NULL)
    {
        printf("%s", error->message);
        g_error_free(error);
        goto out;
    }

    // 处理方法调用的返回值
    g_variant_get(result, "(i)", &response);

out:
    if (result)
        g_variant_unref(result);
    if (proxy)
        g_object_unref(proxy);
    if (connection)
        g_object_unref(connection);
    if (response)
    {
        g_hash_table_replace(schema_key, "custom_value", text);

        GtkTreeIter iter;
        GtkListStore *store = (GtkListStore*)gtk_tree_view_get_model(GTK_TREE_VIEW(table_configure));
        gtk_list_store_clear(store);

        char *key;
        GHashTable *value;
        GHashTableIter iter_hash;
        g_hash_table_iter_init(&iter_hash, schema->values);
        while (g_hash_table_iter_next(&iter_hash, (gpointer*)&key, (gpointer*)&value))
        {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                               COLUMN_NAME, g_hash_table_lookup(value, "node_name"),
                               COLUMN_TYPE, g_hash_table_lookup(value, "value_type"),
                               COLUMN_VALUE, g_hash_table_lookup(value, "custom_value") ? g_hash_table_lookup(value, "custom_value") : g_hash_table_lookup(value, "default_value"),
                               COLUMN_SUMMARY, g_hash_table_lookup(value, "summary"),
                               -1);
        }

        GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_INFO,
                                                   GTK_BUTTONS_OK,
                                                   "设置成功");

        // 设置消息框的标题
        gtk_window_set_title(GTK_WINDOW(dialog), "消息");

        // 显示消息框并等待用户响应
        gtk_dialog_run(GTK_DIALOG(dialog));

        // 销毁消息框
        gtk_widget_destroy(GTK_WIDGET(dialog));
        return;
    }
    else
    {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_INFO,
                                                   GTK_BUTTONS_OK,
                                                   "设置失败");

        // 设置消息框的标题
        gtk_window_set_title(GTK_WINDOW(dialog), "消息");

        // 显示消息框并等待用户响应
        gtk_dialog_run(GTK_DIALOG(dialog));

        // 销毁消息框
        gtk_widget_destroy(GTK_WIDGET(dialog));
    }
}

static void on_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);

    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        GtkTreeSelection *selection = user_data;
        GtkTreeModel *folder_model;
        GtkTreeIter folder_selected;
        if (!gtk_tree_selection_get_selected(selection, &folder_model, &folder_selected))
            return;
        GtkTreePath *path = gtk_tree_model_get_path(folder_model, &folder_selected);
        KSettingsSchema *schema = g_hash_table_lookup(configure_models, gtk_tree_path_to_string(path));

        char *name = NULL;
        gtk_tree_model_get(model, &iter, COLUMN_NAME, &name, -1);
        GHashTable *schema_key = g_hash_table_lookup(schema->values, name);

        char tmp[4096] = "";

        GtkWidget *vbox = gtk_vbox_new(FALSE, 15);

        sprintf(tmp, "%s%s", title_array[0], g_hash_table_lookup(schema_key, "node_name"));
        GtkWidget *name_label = gtk_label_new(tmp);
        gtk_widget_set_halign(name_label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(vbox), name_label, TRUE, TRUE, 0);

        sprintf(tmp, "%s%s", title_array[1], schema->id);
        GtkWidget *id_label = gtk_label_new(tmp);
        gtk_widget_set_halign(id_label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(vbox), id_label, TRUE, TRUE, 0);

        sprintf(tmp, "%s%s", title_array[2], g_hash_table_lookup(schema_key, "summary"));
        GtkWidget *summary_label = gtk_label_new(tmp);
        gtk_widget_set_halign(summary_label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(vbox), summary_label, TRUE, TRUE, 0);

        sprintf(tmp, "%s%s", title_array[3], g_hash_table_lookup(schema_key, "description"));
        GtkWidget *description_label = gtk_label_new(tmp);
        gtk_label_set_line_wrap(GTK_LABEL(description_label), TRUE);
        gtk_widget_set_halign(description_label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(vbox), description_label, TRUE, TRUE, 0);

        sprintf(tmp, "%s%s", title_array[4], g_hash_table_lookup(schema_key, "value_type"));
        GtkWidget *type_label = gtk_label_new(tmp);
        gtk_widget_set_halign(type_label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(vbox), type_label, TRUE, TRUE, 0);

        char *range = g_hash_table_lookup(schema_key, "range");
        if ((NULL != range) && (0 != strcmp("enum", g_hash_table_lookup(schema_key, "value_type"))))
        {
            char **list = g_strsplit(range, ",", -1);
            sprintf(tmp, "%s%s", title_array[5], list[0]);
            GtkWidget *min_tabel = gtk_label_new(tmp);
            gtk_widget_set_halign(min_tabel, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(vbox), min_tabel, TRUE, TRUE, 0);

            sprintf(tmp, "%s%s", title_array[6], list[1]);
            GtkWidget *max_tabel = gtk_label_new(tmp);
            gtk_widget_set_halign(max_tabel, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(vbox), max_tabel, TRUE, TRUE, 0);

            g_strfreev(list);
        }

        sprintf(tmp, "%s%s", title_array[7], g_hash_table_lookup(schema_key, "default_value"));
        GtkWidget *default_label = gtk_label_new(tmp);
        gtk_label_set_line_wrap(GTK_LABEL(default_label), TRUE);
        gtk_widget_set_halign(default_label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(vbox), default_label, TRUE, TRUE, 0);

        sprintf(tmp, "%s%s", title_array[8], g_hash_table_lookup(schema_key, "custom_value") ? g_hash_table_lookup(schema_key, "custom_value") : "默认值");
        GtkWidget *value_label = gtk_label_new(tmp);
        gtk_label_set_line_wrap_mode(GTK_LABEL(value_label), GTK_WRAP_WORD);
        // gtk_label_set_line_wrap(GTK_LABEL(value_label), TRUE);
        gtk_widget_set_halign(value_label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(vbox), value_label, TRUE, TRUE, 0);

        char *writable = g_hash_table_lookup(schema_key, "permission");
        GtkWidget *entry_vbox = gtk_vbox_new(FALSE, 0);
        gtk_widget_set_halign(entry_vbox, TRUE);
        gtk_box_pack_start(GTK_BOX(vbox), entry_vbox, TRUE, TRUE, 0);

        GtkWidget *entry_button = gtk_button_new_with_label("设置自定义值");
        gtk_widget_set_halign(entry_button, GTK_ALIGN_START);
        // name 用来传参。。。。。。
        sprintf(tmp, "%s|%s", gtk_tree_path_to_string(path), g_hash_table_lookup(schema_key, "node_name"));
        gtk_widget_set_name(entry_button, tmp);
        gtk_box_pack_start(GTK_BOX(entry_vbox), entry_button, TRUE, TRUE, 0);

        GtkWidget *text_view = gtk_text_view_new();
        gtk_widget_set_halign(text_view, GTK_ALIGN_START);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
        gtk_box_pack_start(GTK_BOX(vbox), text_view, TRUE, TRUE, 0);

        char *custom_value = g_hash_table_lookup(schema_key, "custom_value") ? g_hash_table_lookup(schema_key, "custom_value") : g_hash_table_lookup(schema_key, "default_value");
        GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), custom_value, strlen(custom_value));
        gtk_text_view_set_buffer(GTK_TEXT_VIEW(text_view), GTK_TEXT_BUFFER(buffer));

        if ((NULL != writable) && (0 != strcmp(writable, "public")))
        {
            gtk_widget_set_sensitive(entry_button, FALSE);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
        }
        g_signal_connect(entry_button, "clicked", G_CALLBACK(on_entry_button_clicked), buffer);

        gtk_container_remove(GTK_CONTAINER(right_scrolled_window), GTK_WIDGET(table_configure));
        gtk_container_add(GTK_CONTAINER(right_scrolled_window), GTK_WIDGET(vbox));

        gtk_widget_show_all(GTK_WIDGET(right_scrolled_window));
    }
}

static void on_selection_changed(GtkTreeSelection *selection, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        // 如果有选中的元素，设置为选中状态
        gtk_tree_selection_select_iter(selection, &iter);
    }
    else
        return;

    GList *list = gtk_container_get_children(GTK_CONTAINER(right_scrolled_window));
    for (GList *tmp = list; tmp; tmp = tmp->next)
    {
        if (tmp->data != table_configure)
        {
            gtk_container_remove(GTK_CONTAINER(right_scrolled_window), GTK_WIDGET(tmp->data));
            gtk_container_add(GTK_CONTAINER(right_scrolled_window), GTK_WIDGET(table_configure));
        }
    }
    g_list_free(list);

    GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
    KSettingsSchema *schema = g_hash_table_lookup(configure_models, gtk_tree_path_to_string(path));
    char *select_path = gtk_tree_path_to_string(path);

    char node_path[2048], tmp_path[64];
    memset(node_path, 0, 2048);
    memset(tmp_path, 0, 64);
    char **nodes = g_strsplit(select_path, ":", -1);
    for (int i = 0; nodes[i]; i++)
    {
        GValue node_name = G_VALUE_INIT;
        strcat(tmp_path, nodes[i]);
        gtk_tree_model_get_iter_from_string(model, &iter, tmp_path);
        gtk_tree_model_get_value(model, &iter, 0, &node_name);
        if (0 < strlen(node_path))
            sprintf(node_path, "%s/%s", node_path, g_value_get_string(&node_name));
        else
            strcpy(node_path, g_value_get_string(&node_name));
        g_value_unset(&node_name);
        strcat(tmp_path, ":");
    }
    g_strfreev(nodes);

    gtk_entry_set_text(GTK_ENTRY(data), node_path);

    GtkListStore *store = (GtkListStore*)gtk_tree_view_get_model(GTK_TREE_VIEW(table_configure));
    gtk_list_store_clear(store);

    if (NULL == schema)
        return;

    char *key;
    GHashTable *value;
    GHashTableIter iter_hash;
    g_hash_table_iter_init(&iter_hash, schema->values);
    while (g_hash_table_iter_next(&iter_hash, (gpointer*)&key, (gpointer*)&value))
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           COLUMN_NAME, g_hash_table_lookup(value, "node_name"),
                           COLUMN_TYPE, g_hash_table_lookup(value, "value_type"),
                           COLUMN_VALUE, g_hash_table_lookup(value, "custom_value") ? g_hash_table_lookup(value, "custom_value") : g_hash_table_lookup(value, "default_value"),
                           COLUMN_SUMMARY, g_hash_table_lookup(value, "summary"),
                           -1);
    }
    gtk_widget_show_all(GTK_WIDGET(right_scrolled_window));
}

static void on_back_button_clicked(GtkWidget *widget, gpointer data)
{
    GList *list = gtk_container_get_children(GTK_CONTAINER(right_scrolled_window));
    for (GList *tmp = list; tmp; tmp = tmp->next)
    {
        if (tmp->data != table_configure)
        {
            gtk_container_remove(GTK_CONTAINER(right_scrolled_window), GTK_WIDGET(tmp->data));
            gtk_container_add(GTK_CONTAINER(right_scrolled_window), GTK_WIDGET(table_configure));
        }
    }
    g_list_free(list);
}

static void _find_search_node(char **list, GtkTreePath **path, GtkTreeModel *model, GtkTreeIter *iter)
{
    static int i = 0;
    do
    {
        GValue value = G_VALUE_INIT;
        gtk_tree_model_get_value(model, iter, 0, &value);
        if (0 == strcmp(list[i], g_value_get_string(&value)))
        {
            if (NULL == list[++i])
            {
                *path = gtk_tree_model_get_path(model, iter);
                break;
            }
            GtkTreeIter child;
            if (gtk_tree_model_iter_has_child(model, iter))
            {
                gtk_tree_model_iter_children(model, &child, iter);
                _find_search_node(list, path, model, &child);
            }
            break;
        }
        g_value_unset(&value);
    } while (gtk_tree_model_iter_next(model, iter));
    i = 0;
}

static void on_search_entry_activated(GtkWidget *entry, gpointer user_data)
{
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
    if (g_str_equal("", text))
        return;
    char **list = g_strsplit(text, "/", -1);

    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_folders));
    gtk_tree_model_get_iter_first(model, &iter);

    GtkTreePath *path = NULL;
    _find_search_node(list, &path, model, &iter);

    // 没搜索到目标路径
    if (NULL == path)
    {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_OK,
                                                   "路径错误");

        // 运行消息框
        gtk_dialog_run(GTK_DIALOG(dialog));

        // 销毁消息框
        gtk_widget_destroy(GTK_WIDGET(dialog));

        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_folders));
        if (gtk_tree_selection_get_selected(selection, &model, &iter))
        {
            GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
            char *select_path = gtk_tree_path_to_string(path);

            char node_path[2048], tmp_path[64];
            memset(node_path, 0, 2048);
            memset(tmp_path, 0, 64);
            char **nodes = g_strsplit(select_path, ":", -1);
            for (int i = 0; nodes[i]; i++)
            {
                GValue node_name = G_VALUE_INIT;
                strcat(tmp_path, nodes[i]);
                gtk_tree_model_get_iter_from_string(model, &iter, tmp_path);
                gtk_tree_model_get_value(model, &iter, 0, &node_name);
                if (0 < strlen(node_path))
                    sprintf(node_path, "%s/%s", node_path, g_value_get_string(&node_name));
                else
                    strcpy(node_path, g_value_get_string(&node_name));
                g_value_unset(&node_name);
                strcat(tmp_path, ":");
            }
            g_strfreev(nodes);

            gtk_entry_set_text(GTK_ENTRY(entry), node_path);
        }
        g_strfreev(list);
        return;
    }
    g_strfreev(list);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_folders));
    gtk_tree_view_expand_to_path(GTK_TREE_VIEW(tree_folders), path);
    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_folders), path, NULL, FALSE, 0.0, 0.0);
    gtk_tree_selection_select_path(selection, path);

    gtk_widget_queue_draw(GTK_WIDGET(tree_folders));
}

static void on_folder_selected(GtkFileChooser *chooser, gint response_id, gpointer data)
{
    switch (response_id)
    {
    case GTK_RESPONSE_ACCEPT:
    {
        char *folder_path = gtk_file_chooser_get_filename(chooser);
        int success = kdk_conf2_save_user_configure(folder_path);
        g_free(folder_path);
        if (0 == success)
        {
            GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                       GTK_DIALOG_MODAL,
                                                       GTK_MESSAGE_ERROR,
                                                       GTK_BUTTONS_OK,
                                                       "保存配置数据到文件失败");

            // 运行消息框
            gtk_dialog_run(GTK_DIALOG(dialog));

            // 销毁消息框
            gtk_widget_destroy(GTK_WIDGET(dialog));
        }
    }
    break;
    default:
        break;
    }
    gtk_widget_destroy(GTK_WIDGET(chooser));
}

static void on_save_button_clicked(GtkWidget *widget, gint response_id, gpointer data)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new("选择文件夹",
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                    "取消", GTK_RESPONSE_CANCEL,
                                                    "打开", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    g_signal_connect(dialog, "response", G_CALLBACK(on_folder_selected), NULL);
    gtk_widget_show_all(dialog);
}

static gboolean on_match_selected(GtkEntryCompletion *self, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
    char *value = NULL;
    gtk_tree_model_get(model, iter, 0, &value, -1);
    gtk_entry_set_text(GTK_ENTRY(gtk_entry_completion_get_entry(self)), value);
    g_signal_emit_by_name(gtk_entry_completion_get_entry(self), "activate", user_data);
    return TRUE;
}

static gboolean entry_match_func(GtkEntryCompletion *completion, const char *key, GtkTreeIter *iter, gpointer user_data)
{
    GtkTreeModel *model = gtk_entry_completion_get_model(completion);
    char *value = NULL;
    gtk_tree_model_get(model, iter, 0, &value, -1);

    char *p = value, *q = p;
    for (; *p != '\0'; p++)
    {
        if (*p == '/')
            q = p;
    }
    if (strstr(q, key))
        return TRUE;
    else
        return FALSE;
}

static void creat_configure_widget_recursively(GtkListStore *store_completion, GtkTreeIter *iter_parent, GHashTable *hash_configure, char *id, char *path)
{
    if (NULL == configure_models)
        configure_models = g_hash_table_new(g_str_hash, g_str_equal);

    KSettingsSchema *schema = g_hash_table_lookup(hash_configure, id);
    if (NULL == schema)
        return;

    GtkTreeIter iter, iter_completion;

    char node_path[512];
    memset(node_path, 0, 512);
    for (int i = 1; i <= strlen(path); i += 2)
    {
        char tmp[1024], *value = NULL;
        memset(tmp, 0, 1024);

        strncpy(tmp, path, i);

        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_folders));
        gtk_tree_model_get_iter_from_string(model, &iter, tmp);
        gtk_tree_model_get(model, &iter, 0, &value, -1);
        if (0 < strlen(node_path))
            sprintf(node_path, "%s/%s", node_path, value);
        else
            strcpy(node_path, value);
    }

    gtk_list_store_append(store_completion, &iter_completion);
    gtk_list_store_set(store_completion, &iter_completion, 0, node_path, -1);

    if (NULL != schema->values)
    {
        g_hash_table_insert(configure_models, strdup(path), schema);
    }
    if (NULL != schema->children)
    {
        GtkTreeStore *store = (GtkTreeStore*)gtk_tree_view_get_model(GTK_TREE_VIEW(tree_folders));

        for (guint i = 0; i < schema->children->len; i++)
        {
            char *child = g_array_index(schema->children, char *, i);
            gtk_tree_store_append(store, &iter, iter_parent);
            gtk_tree_store_set(store, &iter, 0, child, -1);

            // 递归读取子节点的子节点
            char child_id[PATH_MAX];
            memset(child_id, 0, PATH_MAX);
            sprintf(child_id, "%s.%s", id, child);

            GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);

            creat_configure_widget_recursively(store_completion, &iter, hash_configure, child_id, gtk_tree_path_to_string(path));
        }
    }
}

static void draw_scrolled_window(GtkTreeStore *store_app, GtkListStore *store_completion)
{
    if (NULL == schemas_table)
        kdk_conf2_schema_update_schemas_table();
    if (NULL == schemas_table)
        exit(0);

    GtkTreeIter iter;

    // 遍历配置的hash表
    GHashTableIter app_table_iter;
    AppData *app_data;
    GHashTable *hash_app;
    g_hash_table_iter_init(&app_table_iter, schemas_table);
    while (g_hash_table_iter_next(&app_table_iter, (gpointer*)&app_data, (gpointer*)&hash_app))
    {
        // app名写入到store_app中
        gtk_tree_store_append(store_app, &iter, NULL);
        gtk_tree_store_set(store_app, &iter, 0, app_data->name, -1);

        GtkTreeIter iter_completion;
        gtk_list_store_append(store_completion, &iter_completion);
        gtk_list_store_set(store_completion, &iter_completion, 0, app_data->name, -1);

        GtkTreeIter iter_version;

        // 遍历应用的版本号
        GHashTableIter hash_table_iter_version;
        VersionData *versino_data;
        GHashTable *hash_configure;
        g_hash_table_iter_init(&hash_table_iter_version, hash_app);
        while (g_hash_table_iter_next(&hash_table_iter_version, (gpointer*)&versino_data, (gpointer*)&hash_configure))
        {
            // 所有版本号写到应用视图中
            gtk_tree_store_append(store_app, &iter_version, &iter);
            gtk_tree_store_set(store_app, &iter_version, 0, versino_data->name, -1);

            GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store_app), &iter_version);
            creat_configure_widget_recursively(store_completion, &iter_version, hash_configure, app_data->name, gtk_tree_path_to_string(path));
        }
    }
}

int main(int argc, char *argv[])
{
    GtkTreeIter iter;

    gtk_init(&argc, &argv);

    // 创建窗口
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // gtk_window_set_title(GTK_WINDOW(window), "conf2-editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(box));

    GtkWidget *search_entry = gtk_search_entry_new();
    gtk_widget_set_size_request(search_entry, -1, 10);
    gtk_box_pack_start(GTK_BOX(box), search_entry, FALSE, FALSE, 0);
    g_signal_connect(search_entry, "activate", G_CALLBACK(on_search_entry_activated), NULL);

    GtkEntryCompletion *completion = gtk_entry_completion_new();
    gtk_entry_completion_set_text_column(GTK_ENTRY_COMPLETION(completion), 0);
    gtk_entry_completion_set_match_func(GTK_ENTRY_COMPLETION(completion), entry_match_func, NULL, NULL);
    gtk_entry_set_completion(GTK_ENTRY(search_entry), GTK_ENTRY_COMPLETION(completion));
    g_signal_connect(completion, "match-selected", G_CALLBACK(on_match_selected), NULL);

    GtkListStore *store_completion = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_entry_completion_set_model(GTK_ENTRY_COMPLETION(completion), GTK_TREE_MODEL(store_completion));
    // g_object_unref(store_completion);

    GtkWidget *splitter = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(box), splitter, TRUE, TRUE, 0);
    // gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(splitter));
    gtk_paned_set_position(GTK_PANED(splitter), 200);

    // 创建左侧滚动视图，存放组件树
    left_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(left_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_paned_add1(GTK_PANED(splitter), GTK_WIDGET(left_scrolled_window));

    // 创建右侧组件树
    tree_folders = gtk_tree_view_new();
    gtk_container_add(GTK_CONTAINER(left_scrolled_window), GTK_WIDGET(tree_folders));

    // 创建列
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Folders", gtk_cell_renderer_text_new(), "text", 0, NULL);
    gtk_tree_view_insert_column(GTK_TREE_VIEW(tree_folders), column, -1);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_folders), TRUE);

    // 设置为单选模式
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_folders));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

    // 创建应用名树形视图的model
    GtkTreeStore *store_app = gtk_tree_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree_folders), GTK_TREE_MODEL(store_app));

    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_paned_add2(GTK_PANED(splitter), GTK_WIDGET(vbox));

    // GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    // gtk_widget_set_size_request(hbox, 10, 10);
    // gtk_widget_set_size_request(hbox, 10, 10);
    // gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_size_request(button_box, 10, 10);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_END);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);

    GtkWidget *save_button = gtk_button_new();
    GtkWidget *save_icon = gtk_image_new_from_icon_name("folder-download-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(save_button), save_icon);
    gtk_widget_set_size_request(save_button, 10, 10);
    gtk_container_add(GTK_CONTAINER(button_box), GTK_WIDGET(save_button));
    g_signal_connect(G_OBJECT(save_button), "clicked", G_CALLBACK(on_save_button_clicked), NULL);

    GtkWidget *back_button = gtk_button_new();
    GtkWidget *arrow_icon = gtk_image_new_from_icon_name("go-previous", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(back_button), arrow_icon);
    gtk_widget_set_size_request(back_button, 10, 10);
    gtk_container_add(GTK_CONTAINER(button_box), GTK_WIDGET(back_button));
    g_signal_connect(G_OBJECT(back_button), "clicked", G_CALLBACK(on_back_button_clicked), NULL);

    // 创建右侧滚动窗口
    right_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(right_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(right_scrolled_window), TRUE, TRUE, 0);

    // 创建表格
    table_configure = gtk_tree_view_new();
    g_object_ref((gpointer)table_configure);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(table_configure), GTK_TREE_VIEW_GRID_LINES_BOTH);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(table_configure), TRUE);
    g_object_ref((gpointer)table_configure);
    gtk_container_add(GTK_CONTAINER(right_scrolled_window), GTK_WIDGET(table_configure));

    // name列
    GtkCellRenderer *renderer_name = gtk_cell_renderer_text_new();
    g_object_set(renderer_name, "wrap-mode", PANGO_WRAP_WORD_CHAR, NULL);

    GtkTreeViewColumn *colunm_name = gtk_tree_view_column_new_with_attributes("Name", renderer_name, "text", COLUMN_NAME, NULL);
    gtk_tree_view_column_set_min_width(colunm_name, 150);
    gtk_tree_view_insert_column(GTK_TREE_VIEW(table_configure), colunm_name, -1);

    // type列
    GtkCellRenderer *renderer_type = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *colunm_type = gtk_tree_view_column_new_with_attributes("Type", renderer_type, "text", COLUMN_TYPE, NULL);
    gtk_tree_view_column_set_min_width(colunm_type, 150);
    gtk_tree_view_insert_column(GTK_TREE_VIEW(table_configure), colunm_type, -1);

    // value列
    GtkCellRenderer *renderer_value = gtk_cell_renderer_text_new();
    g_object_set(renderer_value, "ellipsize", PANGO_ELLIPSIZE_END, "editable", FALSE, NULL);

    GtkTreeViewColumn *colunm_value = gtk_tree_view_column_new_with_attributes("Value", renderer_value, "text", COLUMN_VALUE, NULL);
    gtk_tree_view_column_set_min_width(colunm_value, 150);
    gtk_tree_view_insert_column(GTK_TREE_VIEW(table_configure), colunm_value, -1);

    // description列
    GtkCellRenderer *renderer_summary = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *colunm_summary = gtk_tree_view_column_new_with_attributes("Summary", renderer_summary, "text", COLUMN_SUMMARY, NULL);
    gtk_tree_view_column_set_min_width(colunm_summary, 150);
    gtk_tree_view_insert_column(GTK_TREE_VIEW(table_configure), colunm_summary, -1);

    GtkListStore *store = gtk_list_store_new(COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(table_configure), GTK_TREE_MODEL(store));

    // 监听组件树选择变化的回调函数
    g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(on_selection_changed), search_entry);
    g_signal_connect(G_OBJECT(table_configure), "row-activated", G_CALLBACK(on_row_activated), selection);

    draw_scrolled_window(store_app, store_completion);

    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}
