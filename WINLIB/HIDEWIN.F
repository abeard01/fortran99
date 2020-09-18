	subroutine hidewin ( tpageno )
        implicit integer(a-z)
c
c  hide      - hide my windows
c
        integer imsg(3), length, rmessage(6), status, dummy
c
        imsg(1) = 46                    ! output window
        imsg(2) = identp(dummy)         ! my page zero
	imsg(3) = tpageno		! my page number
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 6, imsg )
        if ( status .eq. 0 ) length = getmes ( rmessage )
        return
        end
