	subroutine disptime ( windowno, xcoord, ycoord, intptr )
        implicit integer(a-z)
c
c  disptime   - disp a time disp item
c
        integer imsg(12), length, rmessage(6), status, dummy
c
        imsg(1) = 24                    ! output window
        imsg(2) = identp(dummy)         ! my page zero
        imsg(3) = windowno              ! window number
	imsg(4) = xcoord
	imsg(5) = ycoord
	imsg(7) = adr(intptr)
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 22, imsg )
        if ( status .eq. 0 ) length = getmes ( rmessage )
        return
        end
