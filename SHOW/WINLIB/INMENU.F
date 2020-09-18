	integer function inmenu ( dummy )
        implicit integer(a-z)
c
c  inmenu     - get menu item
c
        integer imsg(5), length, rmessage(6), status, dummy
	integer *1 menutext(20)
c
        imsg(1) = 10                    ! inmenu
        imsg(2) = identp(dummy)         ! my page zero
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 10, imsg )
        if ( status .eq. 0 ) then 
          length = getmes ( rmessage )
	  inmenu = rmessage(1)
        else
          inmenu = -1
        endif
        return
        end
