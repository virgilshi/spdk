#include "spdk/stdinc.h"
#include "spdk/thread.h"
#include "spdk/ftl.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/log.h"
#include "spdk/string.h"

#include "lib/ftl/ftl_core.h"

static char* g_ftl_name = "nvme0";

struct hello_context_t {
	char *buff;
	char *ftl_name;
	struct spdk_ftl_dev *dev;
	struct spdk_io_channel *ftl_io_channel;
	union {
		struct iovec *iovs;
		int iovcnt;
	} u;

};

static void
hello_ftl_usage(void)
{
	printf(" -b <directftl>			name of the directftl to use\n");
}

static int hello_ftl_parse_arg(int ch, char *arg)
{
	switch (ch)
	{
	case 'b':
		g_ftl_name = arg;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void 
hello_write(void *arg) {
	struct hello_context_t *hello_context = arg;
	int rc = 0;

	SPDK_NOTICELOG("Writing to the Direct-FTL devices\n");

	snprintf(hello_context->buff, 4096 /*flash page size*/, "hello, HUSTer!\n");

	hello_context->u.iovs.iov_base = hello_context->buff;
	hello_context->u.iovs.iov_len = strlen(hello_context->buff) + 1;
	hello_context->u.iovcnt = 1;

	rc = spdk_ftl_write()
}

static void
hello_start(void *arg1) 
{
	struct hello_context_t *hello_context = arg1;
	int rc = 0;

	SPDK_NOTICELOG("Successfully started the application\n");


	hello_context->buff = spdk_dma_zmalloc(4096 /* flash page size*/, 0 /*not align*/, NULL);
	if (!hello_context->buff) {
		SPDK_ERRLOG("Failed to allocate buffer\n");
		spdk_app_stop(-1);
		return;
	}

	// TODO:
	// Open I/O channel
	hello_context->ftl_io_channel = ftl_get_io_channel();

	hello_write(hello_context);
}

int
main(int argc, char **argv)
{
	struct spdk_app_opts opts = {};
	int rc = 0;
	struct hello_context_t hello_context = {};

	spdk_app_opts_init(&opts);
	opts.name = "hello_test";

	if ((rc = spdk_app_parse_args(argc, argv, &opts, "b:", NULL, hello_ftl_parse_arg,
					hello_ftl_usage)) != SPDK_APP_PARSE_ARGS_SUCCESS) {
		exit(1);
	}
	if (opts.config_file == NULL) {
		SPDK_ERRLOG("configfile must be specified using -c <conffile> e.g. -c bdev.conf\n");
		exit(1);
	}
	hello_context.ftl_name = g_ftl_name;

	rc = spdk_app_start(&opts, hello_start, &hello_context);
	if (rc) {
		SPDK_ERRLOG("ERROR starting application\n");
	}

	spdk_dma_free(hello_context.buff);

	spdk_app_fini();

	return rc;
}