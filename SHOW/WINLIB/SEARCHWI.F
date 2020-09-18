        subroutine searchwin ( windowno, wptr )
        implicit integer(a-z)
c
c  searchwin - search for a given window and return first part of
c              window packet in following format:
c
c		wptr(!) = ptr to next window (unused here)
c               wptr(2) = windowno
c               wptr(3) = tlcx
c               wptr(4) = tlcy
c               wptr(5) = brcx
c               wptr(6) = brcy
c               wptr(7) = color (fg top, bg bottom)
c               wptr(8) = priority (for display)
c
        integer wptr(8)
        integer imsg(11), length, rmessage(6), status, dummy

        do 1000 i=1,7
1000    wptr(i) = 0

        imsg(1) = 6                     ! searchwindow opcode
        imsg(2) = identp(dummy)         ! my page zero
        imsg(2) = windowno              ! window number

        serverpag = getserv(dummy)
        status = sendme ( serverpag, 22, imsg )
        if ( status .eq. 0 ) then
          length = getmes ( rmessage )
c
c  if window found, then copy over the relevant information from the
c  windows server
c
          if ( rmessage(1) .ne. 0 ) then
            call pbytec ( wptr, serverpag, rmessage(1)+2, 16 )
          endif
        endif
        return
        end
