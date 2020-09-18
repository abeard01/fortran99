	subroutine pickroll (windowno, xcoord, ycoord, numbchars, intptr,
     +                      textptr, lowlimit, highlimit )
        implicit integer(a-z)
c
c  pickroll   - pick a roll pick item
c
        integer imsg(12), length, rmessage(6), status, dummy
c
        imsg(1) = 15                    ! output window
        imsg(2) = identp(dummy)         ! my page zero
        imsg(3) = windowno              ! window number
	imsg(4) = xcoord
	imsg(5) = ycoord
	imsg(6) = numbchars
	imsg(7) = adr(intptr)
	imsg(8) = buildc(textptr,numbchars,highlimit)
	imsg(10) = lowlimit
	imsg(11) = highlimit
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 22, imsg )
        if ( status .eq. 0 ) length = getmes ( rmessage )
        return
        end
