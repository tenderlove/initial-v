OPENSCAD = /Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD

%.stl: housing.scad
	$(OPENSCAD) -D'rendering="$(basename $@)"' $< -o $@

all: top.stl middle.stl bottom.stl

clean:
	rm -f *.stl
