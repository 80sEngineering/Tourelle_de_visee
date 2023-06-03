all: get_planes

get_planes: get_planes.c get_gps_data.c
	gcc -Wall -lcurl -o get_planes get_planes.c get_gps_data.c

clean:
	rm -f get planes
