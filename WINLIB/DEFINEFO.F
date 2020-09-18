	subroutine definefo ( height, width, fontname, bitmap )
        implicit integer(a-z)
c
c  definefo  - define a new font
c
        integer imsg(7), length, rmessage(6), status, dummy
	integer *1 zeron(20)
c
        imsg(1) = 37                    ! output window
        imsg(2) = identp(dummy)         ! my page zero
	imsg(4) = height
	imsg(5) = width
	call zero ( fontname, zeron )
	imsg(6) = adr(zeron) 
	imsg(7) = adr(bitmap)
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 14, imsg )
        if ( status .eq. 0 ) length = getmes ( rmessage )
        return
        end
