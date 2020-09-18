	subroutine menu ( column, row, title )
        implicit integer(a-z)
c
c  menu     - create menu item
c
        integer imsg(5), length, rmessage(6), status, dummy
	integer *1 menutext(20)
c
        imsg(1) = 9                     ! menu
        imsg(2) = identp(dummy)         ! my page zero
	imsg(3) = column
	imsg(4) = row
	call zero ( title, menutext )
	imsg(5) = adr(menutext)
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 10, imsg )
        if ( status .eq. 0 ) length = getmes ( rmessage )
        return
        end
