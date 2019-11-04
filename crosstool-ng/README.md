## crosstool-NG Toolchain Configuration

In this repository, you'll find a 

 * [.config](.config): Toolchain configuration for [crosstool-ng](http://crosstool-ng.github.io/) for creating a GCC 5.4 cross compiler targeting our Ubuntu servers.
 * [ct-config](ct-config): Utility script wrapping [crosstool-ng](http://crosstool-ng.github.io/)'s ``menuconfig`` command. Post-processes the configuration in order to disable some gettext and iconv stuff.
