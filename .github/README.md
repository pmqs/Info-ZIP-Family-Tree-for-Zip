# Info-ZIP Family Tree for Zip

This repo combines a number of publically available sources for the [Info-ZIP](https://infozip.sourceforge.net/) program `zip` into a single git repo.
The intention is to create a *family tree* of the many version of the source code that are available.

The base and trunk of the tree contain all the Info-ZIP official and beta releases for `zip` - this is stored in the `infozip` branch of this repo. The branches off `infozip` consist of the main Linux distributions and other forks that have used the Info-ZIP source as their starting point. Each of these is stored in a distinct git branch.


There are no guarantees that this repo contains *all* the available sources for the `zip` code. The Linux distributions in particular (mostly) contain  the same set of changes so it is unlikely they will all ever be included here. If there is an important fork that is missing please report it.

A companion repo  [Info-ZIP-Family-Tree-for-UnZip](https://github.com/pmqs/Info-ZIP-Family-Tree-for-UnZip) contains the equivalent for the Info-ZIP `unzip` program.


## Official & Beta Releases

Below are the sites where official releases and beta source releases were copied from. As alreay mentioned, these sites are merged here into a single git branch called `infozip`.

An attempt has been made to create a semi-accurate timeline by settimg the git timestamps to match the original changes when checking  into this repo. These checkin dates have been sourced from changelogs, when present, and from file timestamps.

When checking the changes into the `infozip` branch the commit messages for the changes  show the origin of each change. Where the Sourceforge site and antinode have the same  files, the Sourceforge copy was used.

* The [Info-ZIP Project](https://sourceforge.net/projects/infozip/) at SourceForge

  This is the primary official site for Info-ZIP. The source files come from https://sourceforge.net/projects/infozip/files. This site contains both official & beta releases of `zip` and `unzip`.

  An older site for official Info-ZIP releases is ftp://ftp.info-zip.org/pub/infozip/src/. The SourceForge site is newer and contains more beta beta versions.


* [antinode.info](http://antinode.info/ftp/info-zip/)

  This site contains a *lot* of beta releases. Most seem to be focused on VMS changes.

**A word of caution:** All the forks off the `infozip`` branch use the zip 3.0 release as their starting point. There are a lot of infozip 3.1 beta relases in the repo that should be considered beta quality. Caveat Emptor.

## Linux and Other OS Distributions

Most Linux/OS distributions ship with a copy Info-ZIP `zip` already installed or have the option to add it. The changes made in these distributions are mostly, but not exclusively, to do with  fixing security and coring issues.


There are a *lot* of distributions out there. Patches from some of the main distributions are shown in the table below -- these seem to be the repos where a lot of other distributions source their changes. There is a huge amount of overlap between these distributions

Each distribution is checked into a branch that matches the distribution name. All are branched from the `3.0` tag in the  `infozip` branch.


| Branch Name | Sourced From | Branched From Infozip |
|---|---|---|
| debian | https://packages.debian.org/source/sid/zip | 3.0 |
| ubuntu | https://packages.ubuntu.com/mantic/zip | 3.0 |
| centos | https://gitlab.com/redhat/centos-stream/rpms/zip | 3.0 |
| fedora | https://src.fedoraproject.org/rpms/zip.git | 3.0 |
| oracle | https://github.com/oracle/solaris-userland/tree/master/components/zip | 3.0 |
| opensuse | https://build.opensuse.org/package/show/Archiving/zip | 3.0 |
| slackware | https://slackware.osuosl.org/slackware-14.2/patches/source/infozip/ | 3.0 |


## Miscellaneous Distributions

Here  are a few repos that have made custom changes to `zip`

| Branch Name | Code Sourced From | Notes | Branched From Infozip Tag |
|---| --- | ---| ---|
| Redfoxymoon-infozip | https://github.com/Redfoxymoon/infozip | Updated Makefile | 3.0
| shelnutt2-zip | https://github.com/Shelnutt2/zip | Android Build | 3.0
| bitwiseworks-unzip-os2 | https://github.com/bitwiseworks/unzip-os2 | OS2 | 3.0
| luadist-zip | https://github.com/LuaDist/zip | CMake & Travis Build| 3.0|
| distropatches-zip-master | https://github.com/distropatches/zip master| Fork from luadist-zip. Documents `--strip-extra` option | 3.0
| distropatches-zip-opensuse | https://github.com/distropatches/zip opensuse |Fork from luadist-zip. Include some opensuse fixes. | 3.0
| distropatches-zip-orig | https://github.com/distropatches/zip orig | Fork from luadist-zip. Includes Info-ZIP 3.1a, b & c.| 3.0



## Forks not included in this repo

These sites below either contain binaries only or have made changes to the infozip sources but in a way that makes it difficult to metge into this repo. They are included here for reference.

| Source | Notes | Branched From Infozip Tag |
| --- | ---| ---|
| https://github.com/jhamby/gnv-zip | VMS Changes | 3.0
| http://hpux.connect.org.uk/hppd/hpux/Misc/zip-3.0/ | HP-UX binaries | 3.0
| https://gitlab.com/FreeDOS/archiver/unzip | FreeDOS | 3.0
| https://gitlab.com/WestCoastRomS/android_external_zip | Android build | 3.0


# AUTHOR

Paul Marquess {pmqs@cpan.org)