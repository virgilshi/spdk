
#include "spdk/stdinc.h"

#include "spdk/env.h"
#include "spdk/thread.h"

#include "spdk/ftl.h"

#include "spdk_internal/event.h"
#include "spdk/env.h"

int spdk_directflt_register(struct spdk_ftl)
static void 
spdk_ftl_initialize_complete(void *cb_arg, int rc) 
{
    spdk_subsystem_init_next(rc);
}

static int 
direct_ftl_read_direct_config(struct spdk_conf_section *sp,
                struct spdk_ftl_dev_init_opts *opts)
{
    const char *val, *trid;
    int rc = 0;

    trid = val = spdk_conf_section_get_nmval(sp, "TransportID", i, 0); // trid
    if (!val) {
        break;
    }

    rc = spdk_nvme_transport_id_parse(&opts->trid, val);
    if (rc < 0) {
        SPDK_ERRLOG("Unable to parse TransportID: %s\n", trid);
        rc = -1;
        goto out;
    }

    if (opts->trid.trtype != SPDK_NVME_TRANSPORT_PCIE) {
        SPDK_ERRLOG("Unsupported transport type for TransportID: %s\n", trid);
        rc = -1;
        goto out;
    }

    val = spdk_conf_section_get_nmval(sp, "TransportID", i, 1);
    if (!val) {
        SPDK_ERRLOG("No name provided for TransportID: %s\n", trid);
        rc = -1;
        goto out;
    }
    
    opts->name = val;

    val = spdk_conf_section_get_nmval(sp, "TransportID", i, 2);
    if (!val) {
        SPDK_ERRLOG("No punit range provided for TransportID: %s\n", trid);
        rc = -1;
        goto out;
    }

    if (bdev_ftl_parse_punits(&opts->range, val)) {
        SPDK_ERRLOG("Invalid punit range for TransportID: %s\n", trid);
        rc = -1;
        break;
    }

    val = spdk_conf_section_get_nmval(sp, "TransportID", i, 3);
    if (!val) {
        SPDK_ERRLOG("No UUID provided for TransportID: %s\n", trid);
        rc = -1;
        break;
    }

    rc = spdk_uuid_parse(&opts->uuid, val);
    if (rc < 0) {
        SPDK_ERRLOG("Failed to parse uuid: %s for TransportID: %s\n", val, trid);
        rc = -1;
        break;
    }

    if (spdk_mem_all_zero(&opts->uuid, sizeof(opts->uuid))) {
        opts->mode = SPDK_FTL_MODE_CREATE;
    } else {
        opts->mode = 0;
    }

out:
    return rc;
}

static void 
direct_ftl_create_cb(void) 
{
    // TODO
    return ;
}

static int 
direct_ftl_create(struct spdk_ftl_dev_init_opts *opts) 
{
    int rc = 0;
    
    if (!opts) {
        SPDK_ERRLOG("opts is NULL\n");
        rc = 1;
        goto error;
    }

    opts->core_thread = opts->read_thread = spdk_get_thread();

    rc = spdk_ftl_dev_init(opts, direct_ftl_create_cb, NULL);
    if (rc) {
        SPDK_ERRLOG("Could not create direct FTL\n");
        rc = 1;
        goto error;
    }
   
error:
    return rc;
}

static void
direct_ftl_initialize_cb(void) {
    struct spdk_conf_section *sp;
    struct spdk_ftl_dev_init_opts *opts = NULL;

    opts = calloc(1, sizeof(*opts));
    if (!opts) {
        SPDK_ERRLOG("Failed to allocate direct ftl init opts\n");
    }
    if (status) {
        SPDK_ERRLOG("Failed to initialize FTL module\n");
        goto out;
    }

    sp = spdk_conf_find_section(NULL, "Ftl");
    if (!sp) {
        goto out;
    }

    spdk_ftl_conf_init_defaults(&opts->ftl_conf);

    if (direct_ftl_read_direct_config(sp, opts)) {
        goto out;
    }

    if (direct_ftl_create(opts)) {
        goto out;
    }

    
out:
    free(opts);

}

static void
spdk_ftl_subsystem_initialize(void) 
{
    // spdk_ftl_initialize(spdk_ftl_initialize_complete, NULL);
    struct ftl_module_init_opts ftl_opts = {};
    ftl_opts.anm_thread = spdk_get_thread();
    rc = spdk_ftl_module_init(&ftl_opts, direct_ftl_initialize_cb, NULL);

    if (rc) {
        SPDK_ERRLOG("Failed to initialize FTL module\n");
        assert(0);
    }
    
}

static void
spdk_ftl_subsystem_finish_done(void *cb_arg)
{
    spdk_subsystem_fini_next();
}

static void
spdk_ftl_subsystem_finish(void) 
{
    spdk_ftl_finish(spdk_ftl_subsystem_finish_done, NULL);
}

static struct spdk_subsystem g_spdk_subsystem_ftl = {
    .name = "directftl",
    .init = spdk_ftl_subsystem_initialize,
    .fini = spdk_ftl_subsystem_finish,
};

SPDK_SUBSYSTEM_REGISTER(g_spdk_subsystem_ftl); //// register ftl module