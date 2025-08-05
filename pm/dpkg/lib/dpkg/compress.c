/*
 * libdpkg - Debian packaging suite library routines
 * compress.c - compression support functions
 *
 * Copyright © 2000 Wichert Akkerman <wakkerma@debian.org>
 * Copyright © 2004 Scott James Remnant <scott@netsplit.com>
 * Copyright © 2006-2023 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <compat.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#if USE_LIBZ_IMPL != USE_LIBZ_IMPL_NONE
#include <compat-zlib.h>
#endif
#ifdef WITH_LIBLZMA
#include <lzma.h>
#endif
#ifdef WITH_LIBZSTD
#include <zstd.h>
#define DPKG_ZSTD_MAX_LEVEL ZSTD_maxCLevel()
#else
#define DPKG_ZSTD_MAX_LEVEL 22
#define ZSTD_CLEVEL_DEFAULT 3
#endif
#define DPKG_ZSTD_CLEVEL_DEFAULT ZSTD_CLEVEL_DEFAULT
#ifdef WITH_LIBBZ2
#include <bzlib.h>
#endif

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/error.h>
#include <dpkg/varbuf.h>
#include <dpkg/fdio.h>
#include <dpkg/buffer.h>
#include <dpkg/meminfo.h>
#include <dpkg/command.h>
#include <dpkg/compress.h>
#if USE_LIBZ_IMPL == USE_LIBZ_IMPL_NONE || \
    !defined(WITH_LIBLZMA) || \
    !defined(WITH_LIBZSTD) || \
    !defined(WITH_LIBBZ2)
#include <dpkg/subproc.h>

static void
fd_fd_filter(struct command *cmd, int fd_in, int fd_out, const char *delenv[])
{
	pid_t pid;

	pid = subproc_fork();
	if (pid == 0) {
		int i;

		if (fd_in != 0) {
			m_dup2(fd_in, 0);
			close(fd_in);
		}
		if (fd_out != 1) {
			m_dup2(fd_out, 1);
			close(fd_out);
		}

		for (i = 0; delenv[i]; i++)
			unsetenv(delenv[i]);

		command_exec(cmd);
	}
	subproc_reap(pid, cmd->name, 0);
}

static void
command_compress_init(struct command *cmd, const char *name, const char *desc,
                      int level)
{
	static char combuf[6];

	command_init(cmd, name, desc);
	command_add_arg(cmd, name);

	snprintf(combuf, sizeof(combuf), "-c%d", level);
	command_add_arg(cmd, combuf);
}

static void
command_decompress_init(struct command *cmd, const char *name, const char *desc)
{
	command_init(cmd, name, desc);
	command_add_arg(cmd, name);
	command_add_arg(cmd, "-dc");
}
#endif

#if defined(WITH_LIBLZMA) || defined(WITH_LIBZSTD)
enum dpkg_stream_filter {
	DPKG_STREAM_COMPRESS	= 1,
	DPKG_STREAM_DECOMPRESS	= 2,
};

enum dpkg_stream_action {
	DPKG_STREAM_INIT	= 0,
	DPKG_STREAM_RUN		= 1,
	DPKG_STREAM_FINISH	= 2,
};

enum dpkg_stream_status {
	DPKG_STREAM_OK,
	DPKG_STREAM_END,
	DPKG_STREAM_ERROR,
};
#endif

struct compressor {
	const char *name;
	const char *extension;
	int default_level;
	void (*fixup_params)(struct compress_params *params);
	void (*compress)(struct compress_params *params,
	                 int fd_in, int fd_out, const char *desc);
	void (*decompress)(struct compress_params *params,
	                   int fd_in, int fd_out, const char *desc);
};

/*
 * No compressor (pass-through).
 */

static void
fixup_none_params(struct compress_params *params)
{
}

static void
decompress_none(struct compress_params *params, int fd_in, int fd_out,
                const char *desc)
{
	struct dpkg_error err;

	if (fd_fd_copy(fd_in, fd_out, -1, &err) < 0)
		ohshit(_("%s: pass-through copy error: %s"), desc, err.str);
}

static void
compress_none(struct compress_params *params, int fd_in, int fd_out,
              const char *desc)
{
	struct dpkg_error err;

	if (fd_fd_copy(fd_in, fd_out, -1, &err) < 0)
		ohshit(_("%s: pass-through copy error: %s"), desc, err.str);
}

static const struct compressor compressor_none = {
	.name = "none",
	.extension = "",
	.default_level = 0,
	.fixup_params = fixup_none_params,
	.compress = compress_none,
	.decompress = decompress_none,
};

/*
 * Gzip compressor.
 */

#define GZIP		"gzip"

static void
fixup_gzip_params(struct compress_params *params)
{
	/* Normalize compression level. */
	if (params->level == 0)
		params->type = COMPRESSOR_TYPE_NONE;
}

#if USE_LIBZ_IMPL != USE_LIBZ_IMPL_NONE
static void
decompress_gzip(struct compress_params *params, int fd_in, int fd_out,
                const char *desc)
{
	char *buffer;
	size_t bufsize = DPKG_BUFFER_SIZE;
	int z_errnum;
	gzFile gzfile = gzdopen(fd_in, "r");

	if (gzfile == NULL)
		ohshit(_("%s: error binding input to gzip stream"), desc);

	buffer = m_malloc(bufsize);

	for (;;) {
		int actualread, actualwrite;

		actualread = gzread(gzfile, buffer, bufsize);
		if (actualread < 0) {
			const char *errmsg = gzerror(gzfile, &z_errnum);

			if (z_errnum == Z_ERRNO)
				errmsg = strerror(errno);
			ohshit(_("%s: internal gzip read error: '%s'"), desc,
			       errmsg);
		}
		if (actualread == 0) /* EOF. */
			break;

		actualwrite = fd_write(fd_out, buffer, actualread);
		if (actualwrite != actualread)
			ohshite(_("%s: internal gzip write error"), desc);
	}

	free(buffer);

	z_errnum = gzclose(gzfile);
	if (z_errnum) {
		const char *errmsg;

		if (z_errnum == Z_ERRNO)
			errmsg = strerror(errno);
		else
			errmsg = zError(z_errnum);
		ohshit(_("%s: internal gzip read error: %s"), desc, errmsg);
	}

	if (close(fd_out))
		ohshite(_("%s: internal gzip write error"), desc);
}

static void
compress_gzip(struct compress_params *params, int fd_in, int fd_out,
              const char *desc)
{
	char *buffer;
	char combuf[6];
	size_t bufsize = DPKG_BUFFER_SIZE;
	int strategy;
	int z_errnum;
	gzFile gzfile;

	if (params->strategy == COMPRESSOR_STRATEGY_FILTERED)
		strategy = 'f';
	else if (params->strategy == COMPRESSOR_STRATEGY_HUFFMAN)
		strategy = 'h';
	else if (params->strategy == COMPRESSOR_STRATEGY_RLE)
		strategy = 'R';
	else if (params->strategy == COMPRESSOR_STRATEGY_FIXED)
		strategy = 'F';
	else
		strategy = ' ';

	snprintf(combuf, sizeof(combuf), "w%d%c", params->level, strategy);
	gzfile = gzdopen(fd_out, combuf);
	if (gzfile == NULL)
		ohshit(_("%s: error binding output to gzip stream"), desc);

	buffer = m_malloc(bufsize);

	for (;;) {
		int actualread, actualwrite;

		actualread = fd_read(fd_in, buffer, bufsize);
		if (actualread < 0)
			ohshite(_("%s: internal gzip read error"), desc);
		if (actualread == 0) /* EOF. */
			break;

		actualwrite = gzwrite(gzfile, buffer, actualread);
		if (actualwrite != actualread) {
			const char *errmsg = gzerror(gzfile, &z_errnum);

			if (z_errnum == Z_ERRNO)
				errmsg = strerror(errno);
			ohshit(_("%s: internal gzip write error: '%s'"), desc,
			       errmsg);
		}
	}

	free(buffer);

	z_errnum = gzclose(gzfile);
	if (z_errnum) {
		const char *errmsg;

		if (z_errnum == Z_ERRNO)
			errmsg = strerror(errno);
		else
			errmsg = zError(z_errnum);
		ohshit(_("%s: internal gzip write error: %s"), desc, errmsg);
	}
}
#else
static const char *env_gzip[] = { "GZIP", NULL };

static void
decompress_gzip(struct compress_params *params, int fd_in, int fd_out,
                const char *desc)
{
	struct command cmd;

	command_decompress_init(&cmd, GZIP, desc);

	fd_fd_filter(&cmd, fd_in, fd_out, env_gzip);

	command_destroy(&cmd);
}

static void
compress_gzip(struct compress_params *params, int fd_in, int fd_out,
              const char *desc)
{
	struct command cmd;

	command_compress_init(&cmd, GZIP, desc, params->level);
	command_add_arg(&cmd, "-n");

	fd_fd_filter(&cmd, fd_in, fd_out, env_gzip);

	command_destroy(&cmd);
}
#endif

static const struct compressor compressor_gzip = {
	.name = "gzip",
	.extension = ".gz",
	.default_level = 9,
	.fixup_params = fixup_gzip_params,
	.compress = compress_gzip,
	.decompress = decompress_gzip,
};

/*
 * Bzip2 compressor.
 */

#define BZIP2		"bzip2"

static void
fixup_bzip2_params(struct compress_params *params)
{
	/* Normalize compression level. */
	if (params->level == 0)
		params->level = 1;
}

#ifdef WITH_LIBBZ2
static void
decompress_bzip2(struct compress_params *params, int fd_in, int fd_out,
                 const char *desc)
{
	char *buffer;
	size_t bufsize = DPKG_BUFFER_SIZE;
	BZFILE *bzfile = BZ2_bzdopen(fd_in, "r");

	if (bzfile == NULL)
		ohshit(_("%s: error binding input to bzip2 stream"), desc);

	buffer = m_malloc(bufsize);

	for (;;) {
		int actualread, actualwrite;

		actualread = BZ2_bzread(bzfile, buffer, bufsize);
		if (actualread < 0) {
			int bz_errnum = 0;
			const char *errmsg = BZ2_bzerror(bzfile, &bz_errnum);

			if (bz_errnum == BZ_IO_ERROR)
				errmsg = strerror(errno);
			ohshit(_("%s: internal bzip2 read error: '%s'"), desc,
			       errmsg);
		}
		if (actualread == 0) /* EOF. */
			break;

		actualwrite = fd_write(fd_out, buffer, actualread);
		if (actualwrite != actualread)
			ohshite(_("%s: internal bzip2 write error"), desc);
	}

	free(buffer);

	BZ2_bzclose(bzfile);

	if (close(fd_out))
		ohshite(_("%s: internal bzip2 write error"), desc);
}

static void
compress_bzip2(struct compress_params *params, int fd_in, int fd_out,
               const char *desc)
{
	char *buffer;
	char combuf[6];
	size_t bufsize = DPKG_BUFFER_SIZE;
	int bz_errnum;
	BZFILE *bzfile;

	snprintf(combuf, sizeof(combuf), "w%d", params->level);
	bzfile = BZ2_bzdopen(fd_out, combuf);
	if (bzfile == NULL)
		ohshit(_("%s: error binding output to bzip2 stream"), desc);

	buffer = m_malloc(bufsize);

	for (;;) {
		int actualread, actualwrite;

		actualread = fd_read(fd_in, buffer, bufsize);
		if (actualread < 0)
			ohshite(_("%s: internal bzip2 read error"), desc);
		if (actualread == 0) /* EOF. */
			break;

		actualwrite = BZ2_bzwrite(bzfile, buffer, actualread);
		if (actualwrite != actualread) {
			const char *errmsg = BZ2_bzerror(bzfile, &bz_errnum);

			if (bz_errnum == BZ_IO_ERROR)
				errmsg = strerror(errno);
			ohshit(_("%s: internal bzip2 write error: '%s'"), desc,
			       errmsg);
		}
	}

	free(buffer);

	BZ2_bzWriteClose(&bz_errnum, bzfile, 0, NULL, NULL);
	if (bz_errnum != BZ_OK) {
		const char *errmsg = _("unexpected bzip2 error");

		if (bz_errnum == BZ_IO_ERROR)
			errmsg = strerror(errno);
		ohshit(_("%s: internal bzip2 write error: '%s'"), desc,
		       errmsg);
	}

	/* Because BZ2_bzWriteClose has done a fflush on the file handle,
	 * doing a close on the file descriptor associated with it should
	 * be safe™. */
	if (close(fd_out))
		ohshite(_("%s: internal bzip2 write error"), desc);
}
#else
static const char *env_bzip2[] = { "BZIP", "BZIP2", NULL };

static void
decompress_bzip2(struct compress_params *params, int fd_in, int fd_out,
                 const char *desc)
{
	struct command cmd;

	command_decompress_init(&cmd, BZIP2, desc);

	fd_fd_filter(&cmd, fd_in, fd_out, env_bzip2);

	command_destroy(&cmd);
}

static void
compress_bzip2(struct compress_params *params, int fd_in, int fd_out,
               const char *desc)
{
	struct command cmd;

	command_compress_init(&cmd, BZIP2, desc, params->level);

	fd_fd_filter(&cmd, fd_in, fd_out, env_bzip2);

	command_destroy(&cmd);
}
#endif

static const struct compressor compressor_bzip2 = {
	.name = "bzip2",
	.extension = ".bz2",
	.default_level = 9,
	.fixup_params = fixup_bzip2_params,
	.compress = compress_bzip2,
	.decompress = decompress_bzip2,
};

/*
 * Xz compressor.
 */

#define XZ		"xz"

#ifdef WITH_LIBLZMA
struct io_lzma {
	const char *desc;

	struct compress_params *params;

	enum dpkg_stream_filter filter;
	enum dpkg_stream_action action;
	enum dpkg_stream_status status;

	void (*init)(struct io_lzma *io, lzma_stream *s);
	void (*code)(struct io_lzma *io, lzma_stream *s);
	void (*done)(struct io_lzma *io, lzma_stream *s);
};

/* XXX: liblzma does not expose error messages. */
static const char *
dpkg_lzma_strerror(struct io_lzma *io, lzma_ret code)
{
	const char *const impossible = _("internal error (bug)");

	switch (code) {
	case LZMA_MEM_ERROR:
		return strerror(ENOMEM);
	case LZMA_MEMLIMIT_ERROR:
		if (io->action == DPKG_STREAM_RUN)
			return _("memory usage limit reached");
		return impossible;
	case LZMA_OPTIONS_ERROR:
		if (io->filter == DPKG_STREAM_COMPRESS &&
		    io->action == DPKG_STREAM_INIT)
			return _("unsupported compression preset");
		if (io->filter == DPKG_STREAM_DECOMPRESS &&
		    io->action == DPKG_STREAM_RUN)
			return _("unsupported options in file header");
		return impossible;
	case LZMA_DATA_ERROR:
		if (io->action == DPKG_STREAM_RUN)
			return _("compressed data is corrupt");
		return impossible;
	case LZMA_BUF_ERROR:
		if (io->action == DPKG_STREAM_RUN)
			return _("unexpected end of input");
		return impossible;
	case LZMA_FORMAT_ERROR:
		if (io->filter == DPKG_STREAM_DECOMPRESS &&
		    io->action == DPKG_STREAM_RUN)
			return _("file format not recognized");
		return impossible;
	case LZMA_UNSUPPORTED_CHECK:
		if (io->filter == DPKG_STREAM_COMPRESS &&
		    io->action == DPKG_STREAM_INIT)
			return _("unsupported type of integrity check");
		return impossible;
	default:
		return impossible;
	}
}

static void
filter_lzma(struct io_lzma *io, int fd_in, int fd_out)
{
	uint8_t *buf_in;
	uint8_t *buf_out;
	size_t buf_size = DPKG_BUFFER_SIZE;
	lzma_stream s = LZMA_STREAM_INIT;

	buf_in = m_malloc(buf_size);
	buf_out = m_malloc(buf_size);

	s.next_out = buf_out;
	s.avail_out = buf_size;

	io->status = DPKG_STREAM_OK;
	io->action = DPKG_STREAM_INIT;
	io->init(io, &s);
	io->action = DPKG_STREAM_RUN;

	do {
		ssize_t len;

		if (s.avail_in == 0 && io->action != DPKG_STREAM_FINISH) {
			len = fd_read(fd_in, buf_in, buf_size);
			if (len < 0)
				ohshite(_("%s: lzma read error"), io->desc);
			if (len == 0)
				io->action = DPKG_STREAM_FINISH;
			s.next_in = buf_in;
			s.avail_in = len;
		}

		io->code(io, &s);

		if (s.avail_out == 0 || io->status == DPKG_STREAM_END) {
			len = fd_write(fd_out, buf_out, s.next_out - buf_out);
			if (len < 0)
				ohshite(_("%s: lzma write error"), io->desc);
			s.next_out = buf_out;
			s.avail_out = buf_size;
		}
	} while (io->status != DPKG_STREAM_END);

	io->done(io, &s);

	free(buf_in);
	free(buf_out);

	if (close(fd_out))
		ohshite(_("%s: lzma close error"), io->desc);
}

static void DPKG_ATTR_NORET
filter_lzma_error(struct io_lzma *io, lzma_ret ret)
{
	ohshit(_("%s: lzma error: %s"), io->desc,
	       dpkg_lzma_strerror(io, ret));
}

#ifdef HAVE_LZMA_MT_ENCODER
static uint64_t
filter_xz_get_memlimit(void)
{
	uint64_t mt_memlimit;

	/* Ask the kernel what is currently available for us. If this fails
	 * initialize the memory limit to half the physical RAM, or to 128 MiB
	 * if we cannot infer the number. */
	if (meminfo_get_available(&mt_memlimit) < 0) {
		mt_memlimit = lzma_physmem() / 2;
		if (mt_memlimit == 0)
			mt_memlimit = 128 * 1024 * 1024;
	}
	/* Clamp the multi-threaded memory limit to half the addressable
	 * memory on this architecture. */
	if (mt_memlimit > INTPTR_MAX)
		mt_memlimit = INTPTR_MAX;

	return mt_memlimit;
}

static uint32_t
filter_xz_get_cputhreads(struct compress_params *params)
{
	long threads_max;

	threads_max = lzma_cputhreads();
	if (threads_max == 0)
		threads_max = 1;

	if (params->threads_max >= 0)
		return clamp(params->threads_max, 1, threads_max);

	return threads_max;
}
#endif

static void
filter_unxz_init(struct io_lzma *io, lzma_stream *s)
{
#ifdef HAVE_LZMA_MT_DECODER
	lzma_mt mt_options = {
		.flags = 0,
		.block_size = 0,
		.timeout = 0,
		.filters = NULL,
	};
#else
	uint64_t memlimit = UINT64_MAX;
#endif
	lzma_ret ret;

	io->filter = DPKG_STREAM_DECOMPRESS;

#ifdef HAVE_LZMA_MT_DECODER
	mt_options.memlimit_stop = UINT64_MAX;
	mt_options.memlimit_threading = filter_xz_get_memlimit();
	mt_options.threads = filter_xz_get_cputhreads(io->params);

	ret = lzma_stream_decoder_mt(s, &mt_options);
#else
	ret = lzma_stream_decoder(s, memlimit, 0);
#endif
	if (ret != LZMA_OK)
		filter_lzma_error(io, ret);
}

static void
filter_xz_init(struct io_lzma *io, lzma_stream *s)
{
	uint32_t preset;
	lzma_check check = LZMA_CHECK_CRC64;
#ifdef HAVE_LZMA_MT_ENCODER
	uint64_t mt_memlimit;
	lzma_mt mt_options = {
		.flags = 0,
		.block_size = 0,
		.timeout = 0,
		.filters = NULL,
		.check = check,
	};
#endif
	lzma_ret ret;

	io->filter = DPKG_STREAM_COMPRESS;

	preset = io->params->level;
	if (io->params->strategy == COMPRESSOR_STRATEGY_EXTREME)
		preset |= LZMA_PRESET_EXTREME;

#ifdef HAVE_LZMA_MT_ENCODER
	mt_options.preset = preset;
	mt_memlimit = filter_xz_get_memlimit();
	mt_options.threads = filter_xz_get_cputhreads(io->params);

	/* Guess whether we have enough RAM to use the multi-threaded encoder,
	 * and decrease them up to single-threaded to reduce memory usage. */
	for (; mt_options.threads > 1; mt_options.threads--) {
		uint64_t mt_memusage;

		mt_memusage = lzma_stream_encoder_mt_memusage(&mt_options);
		if (mt_memusage < mt_memlimit)
			break;
	}

	ret = lzma_stream_encoder_mt(s, &mt_options);
#else
	ret = lzma_easy_encoder(s, preset, check);
#endif

	if (ret != LZMA_OK)
		filter_lzma_error(io, ret);
}

static void
filter_lzma_code(struct io_lzma *io, lzma_stream *s)
{
	lzma_ret ret;
	lzma_action action;

	if (io->action == DPKG_STREAM_RUN)
		action = LZMA_RUN;
	else if (io->action == DPKG_STREAM_FINISH)
		action = LZMA_FINISH;
	else
		internerr("unknown stream filter action %d\n", io->action);

	ret = lzma_code(s, action);

	if (ret == LZMA_STREAM_END)
		io->status = DPKG_STREAM_END;
	else if (ret != LZMA_OK)
		filter_lzma_error(io, ret);
}

static void
filter_lzma_done(struct io_lzma *io, lzma_stream *s)
{
	lzma_end(s);
}

static void
decompress_xz(struct compress_params *params, int fd_in, int fd_out,
              const char *desc)
{
	struct io_lzma io;

	io.init = filter_unxz_init;
	io.code = filter_lzma_code;
	io.done = filter_lzma_done;
	io.desc = desc;
	io.params = params;

	filter_lzma(&io, fd_in, fd_out);
}

static void
compress_xz(struct compress_params *params, int fd_in, int fd_out,
            const char *desc)
{
	struct io_lzma io;

	io.init = filter_xz_init;
	io.code = filter_lzma_code;
	io.done = filter_lzma_done;
	io.desc = desc;
	io.params = params;

	filter_lzma(&io, fd_in, fd_out);
}
#else
static const char *env_xz[] = { "XZ_DEFAULTS", "XZ_OPT", NULL };

static void
decompress_xz(struct compress_params *params, int fd_in, int fd_out,
              const char *desc)
{
	struct command cmd;
	char *threads_opt = NULL;

	command_decompress_init(&cmd, XZ, desc);

	if (params->threads_max > 0) {
		threads_opt = str_fmt("-T%d", params->threads_max);
		command_add_arg(&cmd, threads_opt);
	}

	fd_fd_filter(&cmd, fd_in, fd_out, env_xz);

	command_destroy(&cmd);
	free(threads_opt);
}

static void
compress_xz(struct compress_params *params, int fd_in, int fd_out,
            const char *desc)
{
	struct command cmd;
	char *threads_opt = NULL;

	command_compress_init(&cmd, XZ, desc, params->level);

	if (params->strategy == COMPRESSOR_STRATEGY_EXTREME)
		command_add_arg(&cmd, "-e");

	if (params->threads_max > 0) {
		/* Do not generate warnings when adjusting memory usage, nor
		 * exit with non-zero due to those not emitted warnings. */
		command_add_arg(&cmd, "--quiet");
		command_add_arg(&cmd, "--no-warn");

		/* Do not let xz fallback to single-threaded mode, to avoid
		 * non-reproducible output. */
		command_add_arg(&cmd, "--no-adjust");

		/* The xz -T1 option selects a single-threaded mode which
		 * generates different output than in multi-threaded mode.
		 * To avoid the non-reproducible output we pass -T+1
		 * (supported with xz >= 5.4.0) to request multi-threaded
		 * mode with a single thread. */
		if (params->threads_max == 1)
			threads_opt = m_strdup("-T+1");
		else
			threads_opt = str_fmt("-T%d", params->threads_max);
		command_add_arg(&cmd, threads_opt);
	}

	fd_fd_filter(&cmd, fd_in, fd_out, env_xz);

	command_destroy(&cmd);
	free(threads_opt);
}
#endif

static const struct compressor compressor_xz = {
	.name = "xz",
	.extension = ".xz",
	.default_level = 6,
	.fixup_params = fixup_none_params,
	.compress = compress_xz,
	.decompress = decompress_xz,
};

/*
 * Lzma compressor.
 */

#ifdef WITH_LIBLZMA
static void
filter_unlzma_init(struct io_lzma *io, lzma_stream *s)
{
	uint64_t memlimit = UINT64_MAX;
	lzma_ret ret;

	io->filter = DPKG_STREAM_DECOMPRESS;

	ret = lzma_alone_decoder(s, memlimit);
	if (ret != LZMA_OK)
		filter_lzma_error(io, ret);
}

static void
filter_lzma_init(struct io_lzma *io, lzma_stream *s)
{
	uint32_t preset;
	lzma_options_lzma options;
	lzma_ret ret;

	io->filter = DPKG_STREAM_COMPRESS;

	preset = io->params->level;
	if (io->params->strategy == COMPRESSOR_STRATEGY_EXTREME)
		preset |= LZMA_PRESET_EXTREME;
	if (lzma_lzma_preset(&options, preset))
		filter_lzma_error(io, LZMA_OPTIONS_ERROR);

	ret = lzma_alone_encoder(s, &options);
	if (ret != LZMA_OK)
		filter_lzma_error(io, ret);
}

static void
decompress_lzma(struct compress_params *params, int fd_in, int fd_out,
                const char *desc)
{
	struct io_lzma io;

	io.init = filter_unlzma_init;
	io.code = filter_lzma_code;
	io.done = filter_lzma_done;
	io.desc = desc;
	io.params = params;

	filter_lzma(&io, fd_in, fd_out);
}

static void
compress_lzma(struct compress_params *params, int fd_in, int fd_out,
              const char *desc)
{
	struct io_lzma io;

	io.init = filter_lzma_init;
	io.code = filter_lzma_code;
	io.done = filter_lzma_done;
	io.desc = desc;
	io.params = params;

	filter_lzma(&io, fd_in, fd_out);
}
#else
static void
decompress_lzma(struct compress_params *params, int fd_in, int fd_out,
                const char *desc)
{
	struct command cmd;

	command_decompress_init(&cmd, XZ, desc);
	command_add_arg(&cmd, "--format=lzma");

	fd_fd_filter(&cmd, fd_in, fd_out, env_xz);

	command_destroy(&cmd);
}

static void
compress_lzma(struct compress_params *params, int fd_in, int fd_out,
              const char *desc)
{
	struct command cmd;

	command_compress_init(&cmd, XZ, desc, params->level);
	command_add_arg(&cmd, "--format=lzma");

	fd_fd_filter(&cmd, fd_in, fd_out, env_xz);

	command_destroy(&cmd);
}
#endif

static const struct compressor compressor_lzma = {
	.name = "lzma",
	.extension = ".lzma",
	.default_level = 6,
	.fixup_params = fixup_none_params,
	.compress = compress_lzma,
	.decompress = decompress_lzma,
};

/*
 * ZStandard compressor.
 */

#define ZSTD		"zstd"

#ifdef WITH_LIBZSTD
struct io_zstd_stream {
	enum dpkg_stream_filter filter;
	enum dpkg_stream_action action;
	enum dpkg_stream_status status;

	union {
		ZSTD_CCtx *c;
		ZSTD_DCtx *d;
	} ctx;

	const uint8_t *next_in;
	size_t avail_in;
	uint8_t *next_out;
	size_t avail_out;
};

struct io_zstd {
	const char *desc;

	struct compress_params *params;

	void (*init)(struct io_zstd *io, struct io_zstd_stream *s);
	void (*code)(struct io_zstd *io, struct io_zstd_stream *s);
	void (*done)(struct io_zstd *io, struct io_zstd_stream *s);
};

static void DPKG_ATTR_NORET
filter_zstd_error(struct io_zstd *io, size_t ret)
{
	ohshit(_("%s: zstd error: %s"), io->desc, ZSTD_getErrorName(ret));
}

static uint32_t
filter_zstd_get_cputhreads(struct compress_params *params)
{
	ZSTD_bounds workers;
	long threads_max = 1;

	/* The shared library has not been built with multi-threading. */
	workers = ZSTD_cParam_getBounds(ZSTD_c_nbWorkers);
	if (workers.upperBound == 0)
		return 1;

#ifdef _SC_NPROCESSORS_ONLN
	threads_max = sysconf(_SC_NPROCESSORS_ONLN);
	if (threads_max < 0)
		return 1;
#endif

	if (params->threads_max >= 0)
		return clamp(params->threads_max, 1, threads_max);

	return threads_max;
}

static size_t
filter_zstd_get_buf_in_size(struct io_zstd_stream *s)
{
	if (s->filter == DPKG_STREAM_DECOMPRESS)
		return ZSTD_DStreamInSize();
	else
		return ZSTD_CStreamInSize();
}

static size_t
filter_zstd_get_buf_out_size(struct io_zstd_stream *s)
{
	if (s->filter == DPKG_STREAM_DECOMPRESS)
		return ZSTD_DStreamOutSize();
	else
		return ZSTD_CStreamOutSize();
}

static void
filter_unzstd_init(struct io_zstd *io, struct io_zstd_stream *s)
{
	s->filter = DPKG_STREAM_DECOMPRESS;
	s->action = DPKG_STREAM_RUN;
	s->status = DPKG_STREAM_OK;

	s->ctx.d = ZSTD_createDCtx();
	if (s->ctx.d == NULL)
		ohshit(_("%s: cannot create zstd decompression context"),
		       io->desc);
}

static void
filter_unzstd_code(struct io_zstd *io, struct io_zstd_stream *s)
{
	ZSTD_inBuffer buf_in = { s->next_in, s->avail_in, 0 };
	ZSTD_outBuffer buf_out = { s->next_out, s->avail_out, 0 };
	size_t ret;

	ret = ZSTD_decompressStream(s->ctx.d, &buf_out, &buf_in);
	if (ZSTD_isError(ret))
		filter_zstd_error(io, ret);

	s->next_in += buf_in.pos;
	s->avail_in -= buf_in.pos;
	s->next_out += buf_out.pos;
	s->avail_out -= buf_out.pos;

	if (ret == 0)
		s->status = DPKG_STREAM_END;
}

static void
filter_unzstd_done(struct io_zstd *io, struct io_zstd_stream *s)
{
	ZSTD_freeDCtx(s->ctx.d);
}

static void
filter_zstd_init(struct io_zstd *io, struct io_zstd_stream *s)
{
	int clevel = io->params->level;
	uint32_t nthreads;
	size_t ret;

	s->filter = DPKG_STREAM_COMPRESS;
	s->action = DPKG_STREAM_RUN;
	s->status = DPKG_STREAM_OK;

	s->ctx.c = ZSTD_createCCtx();
	if (s->ctx.c == NULL)
		ohshit(_("%s: cannot create zstd compression context"),
		       io->desc);

	ret = ZSTD_CCtx_setParameter(s->ctx.c, ZSTD_c_compressionLevel, clevel);
	if (ZSTD_isError(ret))
		filter_zstd_error(io, ret);
	ret = ZSTD_CCtx_setParameter(s->ctx.c, ZSTD_c_checksumFlag, 1);
	if (ZSTD_isError(ret))
		filter_zstd_error(io, ret);

	nthreads = filter_zstd_get_cputhreads(io->params);
	if (nthreads > 1)
		ZSTD_CCtx_setParameter(s->ctx.c, ZSTD_c_nbWorkers, nthreads);
}

static void
filter_zstd_code(struct io_zstd *io, struct io_zstd_stream *s)
{
	ZSTD_inBuffer buf_in = { s->next_in, s->avail_in, 0 };
	ZSTD_outBuffer buf_out = { s->next_out, s->avail_out, 0 };
	ZSTD_EndDirective action;
	size_t ret;

	if (s->action == DPKG_STREAM_FINISH)
		action = ZSTD_e_end;
	else
		action = ZSTD_e_continue;

	ret = ZSTD_compressStream2(s->ctx.c, &buf_out, &buf_in, action);
	if (ZSTD_isError(ret))
		filter_zstd_error(io, ret);

	s->next_in += buf_in.pos;
	s->avail_in -= buf_in.pos;
	s->next_out += buf_out.pos;
	s->avail_out -= buf_out.pos;

	if (s->action == DPKG_STREAM_FINISH && ret == 0)
		s->status = DPKG_STREAM_END;
}

static void
filter_zstd_done(struct io_zstd *io, struct io_zstd_stream *s)
{
	ZSTD_freeCCtx(s->ctx.c);
}

static void
filter_zstd(struct io_zstd *io, int fd_in, int fd_out)
{
	ssize_t buf_in_size;
	ssize_t buf_out_size;
	uint8_t *buf_in;
	uint8_t *buf_out;
	struct io_zstd_stream s = {
		.action = DPKG_STREAM_INIT,
	};

	io->init(io, &s);

	buf_in_size = filter_zstd_get_buf_in_size(&s);
	buf_in = m_malloc(buf_in_size);
	buf_out_size = filter_zstd_get_buf_out_size(&s);
	buf_out = m_malloc(buf_out_size);

	s.next_out = buf_out;
	s.avail_out = buf_out_size;

	do {
		ssize_t len;

		if (s.avail_in == 0 && s.action != DPKG_STREAM_FINISH) {
			len = fd_read(fd_in, buf_in, buf_in_size);
			if (len < 0)
				ohshite(_("%s: zstd read error"), io->desc);
			if (len < buf_in_size)
				s.action = DPKG_STREAM_FINISH;
			s.next_in = buf_in;
			s.avail_in = len;
		}

		io->code(io, &s);

		if (s.avail_out == 0 || s.status == DPKG_STREAM_END) {
			len = fd_write(fd_out, buf_out, s.next_out - buf_out);
			if (len < 0)
				ohshite(_("%s: zstd write error"), io->desc);
			s.next_out = buf_out;
			s.avail_out = buf_out_size;
		}
	} while (s.status != DPKG_STREAM_END);

	io->done(io, &s);

	free(buf_in);
	free(buf_out);

	if (close(fd_out))
		ohshite(_("%s: zstd close error"), io->desc);
}

static void
decompress_zstd(struct compress_params *params, int fd_in, int fd_out,
                const char *desc)
{
	struct io_zstd io;

	io.init = filter_unzstd_init;
	io.code = filter_unzstd_code;
	io.done = filter_unzstd_done;
	io.desc = desc;
	io.params = params;

	filter_zstd(&io, fd_in, fd_out);
}

static void
compress_zstd(struct compress_params *params, int fd_in, int fd_out,
              const char *desc)
{
	struct io_zstd io;

	io.init = filter_zstd_init;
	io.code = filter_zstd_code;
	io.done = filter_zstd_done;
	io.desc = desc;
	io.params = params;

	filter_zstd(&io, fd_in, fd_out);
}
#else
static const char *env_zstd[] = {
	"ZSTD_CLEVEL",
	"ZSTD_NBTHREADS",
	NULL,
};

static void
decompress_zstd(struct compress_params *params, int fd_in, int fd_out,
                const char *desc)
{
	struct command cmd;
	char *threads_opt = NULL;

	command_decompress_init(&cmd, ZSTD, desc);
	command_add_arg(&cmd, "-q");

	if (params->threads_max > 0) {
		threads_opt = str_fmt("-T%d", params->threads_max);
		command_add_arg(&cmd, threads_opt);
	}

	fd_fd_filter(&cmd, fd_in, fd_out, env_zstd);

	command_destroy(&cmd);
	free(threads_opt);
}

static void
compress_zstd(struct compress_params *params, int fd_in, int fd_out,
              const char *desc)
{
	struct command cmd;
	char *threads_opt = NULL;

	command_compress_init(&cmd, ZSTD, desc, params->level);
	command_add_arg(&cmd, "-q");

	if (params->level > 19)
		command_add_arg(&cmd, "--ultra");

	if (params->threads_max > 0) {
		threads_opt = str_fmt("-T%d", params->threads_max);
		command_add_arg(&cmd, threads_opt);
	}

	fd_fd_filter(&cmd, fd_in, fd_out, env_zstd);

	command_destroy(&cmd);
	free(threads_opt);
}
#endif

static const struct compressor compressor_zstd = {
	.name = "zstd",
	.extension = ".zst",
	.default_level = DPKG_ZSTD_CLEVEL_DEFAULT,
	.fixup_params = fixup_none_params,
	.compress = compress_zstd,
	.decompress = decompress_zstd,
};

/*
 * Generic compressor filter.
 */

static const struct compressor *compressor_array[] = {
	[COMPRESSOR_TYPE_NONE] = &compressor_none,
	[COMPRESSOR_TYPE_GZIP] = &compressor_gzip,
	[COMPRESSOR_TYPE_XZ] = &compressor_xz,
	[COMPRESSOR_TYPE_ZSTD] = &compressor_zstd,
	[COMPRESSOR_TYPE_BZIP2] = &compressor_bzip2,
	[COMPRESSOR_TYPE_LZMA] = &compressor_lzma,
};

static const struct compressor *
compressor(enum compressor_type type)
{
	const enum compressor_type max_type = array_count(compressor_array);

	if (type < 0 || type >= max_type)
		internerr("compressor_type %d is out of range", type);

	return compressor_array[type];
}

const char *
compressor_get_name(enum compressor_type type)
{
	return compressor(type)->name;
}

const char *
compressor_get_extension(enum compressor_type type)
{
	return compressor(type)->extension;
}

enum compressor_type
compressor_find_by_name(const char *name)
{
	size_t i;

	for (i = 0; i < array_count(compressor_array); i++)
		if (strcmp(compressor_array[i]->name, name) == 0)
			return i;

	return COMPRESSOR_TYPE_UNKNOWN;
}

enum compressor_type
compressor_find_by_extension(const char *extension)
{
	size_t i;

	for (i = 0; i < array_count(compressor_array); i++)
		if (strcmp(compressor_array[i]->extension, extension) == 0)
			return i;

	return COMPRESSOR_TYPE_UNKNOWN;
}

enum compressor_strategy
compressor_get_strategy(const char *name)
{
	if (strcmp(name, "none") == 0)
		return COMPRESSOR_STRATEGY_NONE;
	if (strcmp(name, "filtered") == 0)
		return COMPRESSOR_STRATEGY_FILTERED;
	if (strcmp(name, "huffman") == 0)
		return COMPRESSOR_STRATEGY_HUFFMAN;
	if (strcmp(name, "rle") == 0)
		return COMPRESSOR_STRATEGY_RLE;
	if (strcmp(name, "fixed") == 0)
		return COMPRESSOR_STRATEGY_FIXED;
	if (strcmp(name, "extreme") == 0)
		return COMPRESSOR_STRATEGY_EXTREME;

	return COMPRESSOR_STRATEGY_UNKNOWN;
}

static void
compressor_fixup_params(struct compress_params *params)
{
	compressor(params->type)->fixup_params(params);

	if (params->level < 0)
		params->level = compressor(params->type)->default_level;
}

bool
compressor_check_params(struct compress_params *params, struct dpkg_error *err)
{
	compressor_fixup_params(params);

	if ((params->type == COMPRESSOR_TYPE_ZSTD &&
	     params->level > DPKG_ZSTD_MAX_LEVEL) ||
	    (params->type != COMPRESSOR_TYPE_ZSTD &&
	     params->level > 9)) {
		dpkg_put_error(err, _("invalid compression level %d"),
		               params->level);
		return false;
	}

	if (params->strategy == COMPRESSOR_STRATEGY_NONE)
		return true;

	if (params->type == COMPRESSOR_TYPE_GZIP &&
	    (params->strategy == COMPRESSOR_STRATEGY_FILTERED ||
	     params->strategy == COMPRESSOR_STRATEGY_HUFFMAN ||
	     params->strategy == COMPRESSOR_STRATEGY_RLE ||
	     params->strategy == COMPRESSOR_STRATEGY_FIXED))
		return true;

	if (params->type == COMPRESSOR_TYPE_XZ &&
	    params->strategy == COMPRESSOR_STRATEGY_EXTREME)
		return true;

	dpkg_put_error(err, _("unknown compression strategy"));
	return false;
}

void
decompress_filter(struct compress_params *params, int fd_in, int fd_out,
                  const char *desc_fmt, ...)
{
	va_list args;
	struct varbuf desc = VARBUF_INIT;

	va_start(args, desc_fmt);
	varbuf_add_vfmt(&desc, desc_fmt, args);
	va_end(args);

	compressor(params->type)->decompress(params, fd_in, fd_out, desc.buf);

	varbuf_destroy(&desc);
}

void
compress_filter(struct compress_params *params, int fd_in, int fd_out,
                const char *desc_fmt, ...)
{
	va_list args;
	struct varbuf desc = VARBUF_INIT;

	va_start(args, desc_fmt);
	varbuf_add_vfmt(&desc, desc_fmt, args);
	va_end(args);

	compressor(params->type)->compress(params, fd_in, fd_out, desc.buf);

	varbuf_destroy(&desc);
}
