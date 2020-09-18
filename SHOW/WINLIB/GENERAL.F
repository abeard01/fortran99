        integer function getserv ( dummy )
        implicit integer(a-z)
c
c  wincalls :   window management package
c
c  A subroutine module that passes along window requests to the SHELL SERVER
c
c  The following calls are supported:
c
c  window routines:
c
c
        integer serverpag
        real    *8 name
        data serverpag / 0 /
        data name / 'WINAPI  ' /
c
        if ( serverpag .eq. 0 ) then
          serverpag = findta ( name )
          if ( serverpag .eq. 0 ) then
            write ( *, 9100 )
9100        format ( ' Missing SERVER task WINAPI')
            stop
          endif
        endif
c
        getserv = serverpag
        return
        end
