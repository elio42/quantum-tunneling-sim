Dependencies: 
- Boost Development
- GNUPlot
- wget

install on fedora:
```bash
sudo dnf install gnuplot boost-devel wget
```

Run the command below in project folder to download the header file into the src folder
```bash
wget https://raw.githubusercontent.com/dstahlke/gnuplot-iostream/master/gnuplot-iostream.h -O src/gnuplot-iostream.h
```
