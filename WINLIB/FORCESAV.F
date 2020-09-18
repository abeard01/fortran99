        subroutine forcesa
        implicit integer(a-z)
c
c  forcesa - force a save of all open windows
c
        integer imsg(11), length, rmessage(6), status, dummy
c
        imsg(1) = 4                     ! force save
        imsg(2) = identp(dummy)         ! my page zero
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 22, imsg )
        if ( status .eq. 0 ) length = getmes ( rmessage )
        return
        end
