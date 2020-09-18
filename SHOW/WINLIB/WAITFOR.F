        integer function waitfore ( dummy )
        implicit integer(a-z)
c
c  waitfore  - wait for any event
c
        integer imsg(2), length, rmessage(6), status, dummy
c
        imsg(1) = 42                    ! output window
        imsg(2) = identp(dummy)         ! my page zero
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 4, imsg )
        if ( status .eq. 0 ) then
	  length = getmes ( rmessage )
	  waitfore = rmessage(1)
        else
	  waitfore = -1
	endif
        return
        end
