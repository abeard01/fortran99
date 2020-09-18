	subroutine wgputs ( windowno, charlen, string )
        implicit integer(a-z)
c
c  wgputs    - write a string to a window
c
        integer imsg(4), length, rmessage(6), status, dummy
	integer *1 zeron(200), string(200)
c
	do 1010 i=1,charlen
1010	zeron(i) = string(i)
	zeron(charlen+1) = 0
c
        imsg(1) = 35                    ! output window
        imsg(2) = identp(dummy)         ! my page zero
	imsg(3) = windowno
	imsg(4) = adr(zeron) 
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 8, imsg )
        if ( status .eq. 0 ) length = getmes ( rmessage )
        return
        end
