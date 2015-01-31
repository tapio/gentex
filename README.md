Texture Generator
=================

This is an incomplete command line tool that generates textures based on a JSON spec.
Parts of the project are inspired by mrdoob's [texgen.js](https://github.com/mrdoob/texgen.js).

## Generator Functions:

* constant color
* random noise
* sin x / sin y
* or / xor

## Tool Features

* TGA output
* Watch files for changes and automatically regenerate textures
* Files can contain multiple output textures

## Usage Example

	$ cat << EOF > test.json
	{
		"size": [ 256, 256 ],
		"out": "test.tga",
		"ops": [
			{ "add": "xor", "tint": [ 1.0, 0.5, 0.7 ] },
			{ "add": "sinx", "freq": 0.004, "tint": [ 0.25, 0.0, 0.0 ] },
			{ "sub": "siny", "freq": 0.004, "tint": [ 0.25, 0.0, 0.0 ] },
			{ "add": "sinx", "freq": 0.0065, "tint": [ 0.1, 0.5, 0.2 ] },
			{ "add": "siny", "freq": 0.0065, "tint": [ 0.0, 0.4, 0.5 ] },
			{ "add": "noise", "tint": [ 0.1, 0.1, 0.2 ] }
		]
	}
	EOF
	
	$ gentex test.json

## Building

You need CMake and a C++11 compiler such as g++ 4.9 or clang++ 3.5. A couple of needed third-party libraries are included in the repository so there are no additional dependencies.

	mkdir build
	cd build
	cmake ..
	make -j4

