	subroutine deletefo ( fontname )
        implicit integer(a-z)
c
c  deletefont   - delete the given font
c
        integer imsg(7), length, rmessage(6), status, dummy
	integer *1 zeron(20)
c
        imsg(1) = 39                    ! delete font
        imsg(2) = identp(dummy)         ! my page zero
	call zero ( fontname, zeron )
	imsg(6) = adr(zeron) 
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 14, imsg )
        if ( status .eq. 0 ) length = getmes ( rmessage )
        return
        end
