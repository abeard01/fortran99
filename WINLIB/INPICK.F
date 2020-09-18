	integer function inpick ( windowno )
        implicit integer(a-z)
c
c  inpick    - wait for a pick item
c
        integer imsg(12), length, rmessage(6), status, dummy
c
        imsg(1) =  33                     ! output window
        imsg(2) = identp(dummy)         ! my page zero
        imsg(3) = windowno              ! window number
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
