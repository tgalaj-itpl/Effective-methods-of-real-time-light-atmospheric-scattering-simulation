## Synopsis

This project implements 8 clear sky models in a common framework to compare
them with each other and with a reference model and reference measurements.

## Cloning
To clone the whole repo use the following command:

```
git clone --recursive [repo_address] [folder_to_store_repo_files]
```

## Libradtran installation
To install libradtran follow the instructions on [this site](http://web.archive.org/web/20180614161856/http://www.inperfectsilence.org/getting-libradtran-up-and-running-on-a-mac/) or [this one](https://www.meteo-blog.net/2013-09/installing-libradtran-on-macbook-air-mountain-lion/).

or run the following commands:

```bash
sudo add-apt-repository universe
sudo apt update
sudo apt install p7zip-full p7zip-rar zlib1g gcc gfortran libnetcdf-dev libnetcdff-dev libgsl23 libgmp3-dev curl perl flex gawk f2c libglm-dev gnuplot libgsl-dev

curl -SLO http://www.libradtran.org/download/libRadtran-2.0.3.tar.gz
gzip -d libRadtran-2.0.3.tar.gz
tar -xvf libRadtran-2.0.3.tar

# Note: Remember to configure python alternatives, so that python command is recognized by the system: https://www.vultr.com/docs/how-to-install-python-2-on-ubuntu-20-04

cd libRadtran-2.0.3
./configure --with-f2c
make
make check

sudo make install
export LIBRADTRAN_DATA_FILES=/usr/local/share/libRadtran/data/

cd ..
rm libRadtran-2.0.3.tar
rm -rf libRadtran-2.0.3
```

To build and run the code use:
```
make all
```

To generate plots run the following command:
```
gnuplot output/figures/main.plot
```

## Results generator
To generate results follow these steps:
1) Run ```output/gen_lab_diffs.bat``` (**NOTICE:** You have to change path to python in the last line of the file)
2) Build PDF with ```results_generator/results-generator.tex```
   
## License

This project is released under the BSD license.
