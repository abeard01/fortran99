	subroutine dispicon (windowno, xcoord, ycoord, numbchars, bitmap,
     +                       textptr )
        implicit integer(a-z)
c
c  dispicon   - disp an icon disp item
c
        integer imsg(12), length, rmessage(6), status, dummy
c
        imsg(1) = 27                    ! output window
        imsg(2) = identp(dummy)         ! my page zero
        imsg(3) = windowno              ! window number
	imsg(4) = xcoord
	imsg(5) = ycoord
	imsg(6) = numbchars
	imsg(8) = adr(textptr)		! text pointer
	imsg(9) = adr(bitmap)		! bit map
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 22, imsg )
        if ( status .eq. 0 ) length = getmes ( rmessage )
        return
        end
