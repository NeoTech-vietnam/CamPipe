# CamPipe

Independent NeoCam module copied from NeoLab firmware.

Implementation, public contracts, module.cmake and owned tests live here.
Product configuration is supplied by neocam_csw/Conf. See the integration
README for host tests, live overrides and publication instructions.

The configured-task pilot requires a compatible EbOs revision and the project's
`Conf/EbOs` configuration. `CamPipe_TaskConfig.h` declares the project-supplied
task descriptor; EbOs owns native task creation, cooperative join/deletion and
stack reporting. Camera work and the stop request remain owned by CamPipe.

## Documentation

Read the [published CamPipe specification](https://neotech-vietnam.github.io/CamPipe/specification.html)
or build the [reStructuredText documentation](docs/index.rst) from this repository.
