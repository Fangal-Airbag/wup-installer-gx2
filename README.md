# WUP Installer GX2 WUHB
This a WUHB port of WUP Installer GX2 which can be found [here](https://sourceforge.net/projects/wup-installer-gx2/)

## Bulding

### Makefile
To build this application using make, you will need [wut](https://github.com/devkitPro/wut) and the following devkitPro packages:
`
ppc-zlib ppc-libmad ppc-libogg ppc-libgd ppc-freetype ppc-brotli ppc-libjpeg-turbo ppc-libpng ppc-libvorbisidec ppc-glm ppc-bzip2
`

Once the dependencies are installed just run `make` and use the resulting `.wuhb` file.

### Dockerfile
To build this application using docker, run the following commands:
```shell
docker buildx build . -t wupgx2 --load
docker run -it --rm -v "$PWD":/app wupgx2
```

You can then use the resulting `.wuhb` file.

# Credits
A big thanks goes out to [brienj](https://github.com/xhp-creations) for creating the original rpx port of WUP Installer GX2,
and [Gary](https://github.com/GaryOderNichts) for making the Wii U controller mod version.

Also to [Yardape8000](https://github.com/Yardape8000) for creating the wup installer y mod which WUP Installer GX2 was based on. [His repo](https://github.com/Yardape8000/wupinstaller).

And of course [dimok](https://github.com/dimok789) who created the original apps HBL and Loadiine which use the same base, 
as well as the original wupinstaller, and [Maschell](https://github.com/Maschell) for creating the new Aroma environment 
and all the other amazing work he's done for the community. Big thanks to them as well. 