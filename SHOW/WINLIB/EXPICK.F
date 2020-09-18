	subroutine expick ( windowno )
        implicit integer(a-z)
c
c  expick    - display the pick items
c
        integer imsg(12), length, rmessage(6), status, dummy
c
        imsg(1) = 34                    ! output window
        imsg(2) = identp(dummy)         ! my page zero
	imsg(3) = windowno
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 22, imsg )
        if ( status .eq. 0 ) then
          length = getmes ( rmessage )
          inpick = rmessage(1)
        else
          inpick = -1
        endif
        return
        end
