# First, define the tmp directory 

all: 	makecorefile \
	stop s g index turn+ turn- turn-dec turn+dec turn-ra turn+ra quit \
	dome \
	fixed zenith service \
	pcat catalog newobject follow \
	offsets nn ss ee ww zero col icol off2 ioff2 setcol setdial \
	dialing keypad setnod nod pad \
	seeking showall track rate info shortinfo \
	filter refresh clrusr fheader \
	slecho spiral trackscreen setfocus tstatus

#all: track follow stop s g index turn+ turn- offsets nn ss ee ww zero \
#	col icol off2 ioff2 fixed zenith service catalog newobject dialing\
#	keypad showall seeking info rate setcol setdial \
#	makecorefile refresh clrusr  turn+ra turn+dec turn-ra turn-dec dome \
#	setnod nod pad filter fheader nmirdfheader


LOG = /home/observer/wiro/bin/log_entry.o


####################
# First, some maintanence things
#####################

backup:
	tar cvf ../newtrack.tar todo *.c *.h makefile   RCS instructions
	gzip ../newtrack.tar

clean:
	-rm -f *.o makecorefile \
	stop s g index turn+ turn- turn-dec turn+dec turn-ra turn+ra quit \
	dome \
	fixed zenith service \
	pcat catalog newobject follow \
	offsets nn ss ee ww zero col icol off2 ioff2 setcol setdial \
	dialing keypad setnod nod pad \
	seeking showall track rate info shortinfo \
	filter refresh clrusr fheader \
	slecho spiral trackscreen setfocus tstatus


#######################################################
# The next few are various testing programs.
################

test1.o: test1.c
	gcc -c   test1.c

test1: test1.o
	gcc -X -m   test1.o -o test1

testport.o: testport.c
	gcc -c -g testport.c
testport: testport.o ../lib/openport.o
	gcc -X -m testport.o ../lib/openport.o -o testport

###################
# Utility routines objects 
###################
times.o: times.c
	gcc -c  times.c
get_tinfo.o: get_tinfo.c wiro.h wtrack.h
	gcc -c get_tinfo.c
oblecl.o: oblecl.c wtrack.h
	gcc  -c oblecl.c
lonobl.o: lonobl.c wtrack.h
	gcc  -c lonobl.c
annabr.o : annabr.c wtrack.h
	gcc  -c  annabr.c
nutation.o: nutation.c wtrack.h
	gcc  -c  nutation.c
julian.o: julian.c wtrack.h
	gcc  -c  julian.c
str2dec.o: str2dec.c
	gcc  -c  str2dec.c
rem.o: rem.c
	gcc  -c  rem.c
angles.o: angles.c 
	gcc  -c  angles.c
hr_hms.o: hr_hms.c wtrack.h
	gcc  -c  hr_hms.c
track.o: track.c  GPIBports.h worm_corr.h wiro.h wtrack.h wirotypes.h \
		parameters.h
	gcc  -c  track.c
ntrack.o: ntrack.c  GPIBports.h worm_corr.h wiro.h wtrack.h wirotypes.h \
		parameters.h
	gcc  -c  ntrack.c
diagnostic.o: diagnostic.c wiro.h parameters.h GPIBports.h wirotypes.h track.h\
		wtrack.h
	gcc  -c  diagnostic.c
tscreen.o: tscreen.c wiro.h  wtrack.h
	gcc  -c tscreen.c
GPIBinit.o: GPIBinit.c GPIBports.h
	gcc  -g -c GPIBinit.c
GPIBports.o: GPIBports.c GPIBports.h wirotypes.h
	gcc  -g -c  GPIBports.c


track: ntrack.o GPIBports.o angles.o tscreen.o hr_hms.o rem.o \
	openport.o times.o diagnostic.o get_tinfo.o $(LOG)
#	gcc -X -m -v track.o GPIBports.o angles.o tscreen.o hr_hms.o rem.o \
#		openport.o \
#		/lib/thread/libgnu.a /lib/thread/init.o /lib/thread/libc_p.a \
#		/lib/thread/libc.a   \
#		-o track 
	gcc  ntrack.o GPIBports.o angles.o tscreen.o hr_hms.o rem.o \
		openport.o times.o  diagnostic.o get_tinfo.o  $(LOG)\
		-lm  -pthread -lc -o track 
		chmod 711 track
		chmod +s track

#######################################################
follow.o: follow.c wiro.h
	gcc  -c follow.c
follow: follow.o str2dec.o $(LOG)
	gcc follow.o str2dec.o get_tinfo.o $(LOG) -o follow -lm
	chmod 711 follow
	chmod +s follow
#######################################################
pcat.o: pcat.c wiro.h
	gcc -c  pcat.c 
pcat: pcat.o str2dec.o
	gcc  pcat.o str2dec.o get_tinfo.o -lm -o pcat 

#######################################################

tstatus.o: tstatus.c wiro.h wirotypes.h
	gcc -c tstatus.c
tstatus: tstatus.o get_tinfo.o
	gcc tstatus.o get_tinfo.o -lm -o tstatus

#######################################################
stop.o: stop.c wiro.h
	gcc -c -g stop.c
stop: stop.o $(LOG)
	gcc  stop.o get_tinfo.o $(LOG) -o stop 
	chmod 711 stop
	chmod +s stop
s: stop
	ln -s stop s 
g: stop
	ln -s stop g 
index: stop
	ln -s stop index 
turn+: stop
	ln  -s stop turn+
turn-: stop
	ln -s stop turn-
turn-dec: stop
	ln -s stop turn-dec
turn+dec: stop
	ln -s stop turn+dec
turn-ra: stop
	ln -s stop turn-ra
turn+ra: stop
	ln -s stop turn+ra
quit: stop
	ln -s stop quit

#######################################################
dome.o : dome.c wiro.h
	gcc -c dome.c
dome: dome.o $(LOG)
	gcc  dome.o  get_tinfo.o $(LOG) -o dome
	chmod 711 dome
	chmod +s dome


#######################################################
offsets.o : offsets.c wiro.h
	gcc  -c offsets.c
offsets: offsets.o $(LOG)
	gcc  offsets.o get_tinfo.o $(LOG) -o offsets
	chmod 1711 offsets
	chmod +s offsets
nn: offsets
	cp offsets nn	
ss: offsets
	cp offsets ss
ee: offsets
	cp offsets ee
ww: offsets
	cp offsets ww	
zero: offsets
	cp offsets zero
col: offsets
	ln -s offsets col
icol: offsets
	ln -s offsets icol
off2: offsets
	ln -s offsets off2
ioff2: offsets
	ln -s offsets ioff2
setcol: offsets
	ln -s offsets setcol
setdial: offsets
	ln -s offsets setdial

#######################################################
fixed.o : fixed.c wiro.h
	gcc  -c fixed.c
fixed: fixed.o $(LOG)
	gcc fixed.o get_tinfo.o $(LOG) -o fixed 
	chmod 711 fixed
	chmod +s fixed
zenith: fixed 
	ln -s fixed zenith
service: fixed
	ln -s fixed service


#######################################################
dialing.o : dialing.c wiro.h
	gcc -c dialing.c
dialing: dialing.o $(LOG)
	gcc dialing.o get_tinfo.o $(LOG) -lm  -o dialing 
	chmod 711 dialing
	chmod +s dialing

#######################################################
catalog.o: catalog.c wiro.h
	gcc -c -g  catalog.c
catalog: catalog.o $(LOG)
	gcc catalog.o $(LOG) -g -lm -o catalog 
	chmod 711 catalog
	chmod +s catalog
newobject: catalog
	ln -s catalog newobject

#######################################################
keypad.o: keypad.c wiro.h wirotypes.h
	gcc -c  keypad.c
keypad: keypad.o $(LOG)
	gcc  keypad.o get_tinfo.o $(LOG) -lm  -o keypad 
	chmod 711 keypad
	chmod +s keypad
#######################################################
pad: pad.o get_tinfo.o $(LOG)
	gcc  -g -o pad pad.o get_tinfo.o $(LOG) -lslang
	chmod 711 pad
	chmod +s pad

pad.o: pad.c wiro.h wirotypes.h
	gcc -g -c  pad.c


slecho: slecho.c
	gcc -o slecho slecho.c -lslang
#######################################################
setfocus: setfocus.o get_tinfo.o $(LOG)
	gcc  -g -o setfocus setfocus.o get_tinfo.o $(LOG) -lslang
	chmod 711 setfocus
	chmod +s setfocus

setfocus.o: setfocus.c wiro.h wirotypes.h
	gcc -g -c  setfocus.c

#######################################################
setnod: setnod.o get_tinfo.o $(LOG)
	gcc -o setnod setnod.o get_tinfo.o $(LOG)
	chmod 711 setnod
	chmod +s setnod

setnod.o: setnod.c wiro.h wirotypes.h
	gcc -c setnod.c

#######################################################
nod: nod.o get_tinfo.o $(LOG)
	gcc  -o nod nod.o get_tinfo.o $(LOG) 
	chmod 711 nod
	chmod +s nod

nod.o: nod.c wiro.h wirotypes.h
	gcc -c nod.c

#######################################################
spiral: spiral.o get_tinfo.o $(LOG)
	gcc  -o spiral spiral.o get_tinfo.o $(LOG) -lslang
	chmod 711 spiral
	chmod +s spiral
spiral.o: spiral.c wiro.h
	gcc -c  spiral.c

#######################################################
rate.o: rate.c wiro.h wirotypes.h
	cc -c  rate.c
rate: rate.o times.o get_tinfo.o $(LOG)
	cc rate.o times.o get_tinfo.o $(LOG) -lm -o rate 
	chmod 711 rate
	chmod +s rate

#######################################################
thereyet.o: seeking.c wiro.h wirotypes.h
	cc -c  thereyet.c
seeking: thereyet.o seeking.c
	cc  seeking.c thereyet.o get_tinfo.o -o seeking

showall: showall.c
	cc -o showall showall.c

#######################################################
info: info.c wiro.h get_tinfo.o
	cc   -o info info.c get_tinfo.o -lm

shortinfo: shortinfo.c wiro.h get_tinfo.o
	cc  -o shortinfo shortinfo.c get_tinfo.o -lm

makecorefile: makecorefile.c wiro.h get_tinfo.o $(LOG)
	cc -o makecorefile makecorefile.c get_tinfo.o $(LOG)
	chmod 711 makecorefile
	chmod +s makecorefile

refresh: refresh.c wiro.h get_tinfo.o
	cc  -o refresh refresh.c get_tinfo.o -lm

clrusr: clrusr.c wiro.h get_tinfo.o
	cc -o clrusr clrusr.c get_tinfo.o -lm

filter: filter.c wiro.h get_tinfo.o
	cc -o filter filter.c get_tinfo.o -lm

fitshead.o: fitshead.c 
	cc -c fitshead.c

fheader: fheader.c fitshead.o
	cc  -o fheader fheader.c fitshead.o get_tinfo.o -lm
	chmod u+s fheader

nmirdfheader: nmirdfheader.c fitshead.o
	cc -g -o nmirdfheader nmirdfheader.c fitshead.o
	chmod u+s nmirdfheader

nmird2fheader:	nmird2fheader.c fitshead.o
	cc -g -o nmird2fheader nmird2fheader.c fitshead.o
	chmod u+s nmird2fheader

threadtest: threadtest.o
	cc -g -o threadtest  threadtest.o -lm -pthread

trackscreen.o: trackscreen.c
	cc -c trackscreen.c
trackscreen: trackscreen.o angles.o tscreen.o hr_hms.o
	cc trackscreen.o angles.o tscreen.o hr_hms.o get_tinfo.o \
	 -o trackscreen -lm
	chmod 711 trackscreen
	chmod +s trackscreen
