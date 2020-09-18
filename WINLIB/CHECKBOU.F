        logical function checkbou ( ix, iy, wptr )
        implicit integer(a-z)
c
c  checkbo - check if mouse click inside this window definition
c
        integer ix, iy, wptr(20)
c
        checkbou = .FALSE.
        if ( ix .lt. wptr(2) ) return
        if ( ix .gt. wptr(4) ) return
        if ( iy .lt. wptr(3) ) return
        if ( iy .gt. wptr(5) ) return
        checkbou = .TRUE.
        return
        end
