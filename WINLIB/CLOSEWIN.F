        subroutine closewin ( windowno )
        implicit integer(a-z)
c
c  closewin - close an open window
c
        integer imsg(11), length, rmessage(6), status, dummy
c
        imsg(1) = 3                     ! close window
        imsg(2) = identp(dummy)         ! my page zero
        imsg(3) = windowno              ! window number
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 22, imsg )
        if ( status .eq. 0 ) length = getmes ( rmessage )
        return
        end
