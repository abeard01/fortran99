	subroutine sharewi
        implicit integer(a-z)
c
c  share     - enable window sharing
c
        integer imsg(3), length, rmessage(6), status, dummy
c
        imsg(1) = 47                    ! output window
        imsg(2) = identp(dummy)         ! my page zero
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 6, imsg )
        if ( status .eq. 0 ) length = getmes ( rmessage )
        return
        end
