        program wintest
        implicit integer(a-z)
c
c  windows test program
c
        real *8 name1, name2, name3, name4, select(8), menn(10)
        integer *1 buffer(16)
        data name1 / 'Test_#1 ' /
        data name2 / 'Test_#3 ' /
        data name3 / 'Test_#4 ' /
        data buffer / 16hTest String       /
        data select / 'Select1 ', 'Select2 ', 'Select3 ', 'Select4 ',
     +                'Select5 ', 'Select6 ', 'Select7 ', 'Select8 ' /
        data menn   / 'Project ', 'About   ', 'Help    ', 'Quit    ',
     +                'A10Char ', 'This is ', 'Another ', 'Menu    ',
     +                'Item    ', 'OK      ' /

        call window ( 1, 10, 20, 200, 100, 15, 4, name1, 0 )
        write ( 6, 9100 )
9100    format ( ' This is a test window')

        call window ( -2, 80, 120, 300, 100, 11, 8, 0, 0 )
        write ( 6, 9110 )
9110    format ( ' This is test window #2')

        call window ( 3, 220, 20, 200, 100, 14, 1, name2, 0 )
        write ( 6, 9120 )
9120    format ( ' This is test window #3')

        call close ( -2 )
        call outputwi ( 1 )

        write ( 6, 9130 )
9130    format ( ' Another write to window 1')

        do 1000 i=1,20000
1000    continue
c
c  test of pick lists
c
        call closewin(1)
        call closewin(2)
        call closewin(3)
c
        call window ( 4, 220, 20, 200, 100, 9, 1, name3, 0 )
        call window ( 5,  10, 20, 200, 100, 9, 1, name4, 0 )
        selint = 1
        selrol = 1
        call pickint ( 4, 1, 1, 6, selint, 1, 5 )
        call picktext( 4, 1, 2, 15, buffer )
        call pickroll( 4, 1, 3, 8, selrol, select, 1, 8 )
        call dispint ( 5, 1, 1, 6, setint )
        call disptext( 5, 1, 2, 15, buffer )
        call disproll( 5, 1, 3, 8, setrol, select )
c
        call expick(5)
c
        j = inpick(4)

        call expick(5)

        call outputwi(4)
        write ( 6, 9140 ) j
9140    format ( ' Returned value for j is ',i6 )

        do 1010 i=1,20000
1010    continue
c
c  test of menu items
c
        call menu ( 1, 0, menn(1) )
        call menu ( 1, 1, menn(2) )
        call menu ( 1, 2, menn(3) )
        call menu ( 1, 3, menn(4) )
        call menu ( 1, 4, menn(5) )
        call menu ( 2, 0, menn(6) )
        call menu ( 2, 1, menn(7) )
        call menu ( 2, 2, menn(8) )
        call menu ( 3, 0, menn(9) )
        call menu ( 3, 1, menn(10) )
c
        write ( 6, 9150 )
9150    format ( ' going to wait for a menu item ' )
c
        j = inmenu(0)
c
        write ( 6, 9160 ) j
9160    format ( ' returned value for menu is ',i6 )
c
c   ok now wait for an event
c
        key = waitfore(dummy)
        write ( 6, 9170 ) key
9170    format( ' Returned key is ',z4 )
c
c  try some window sharing
c
        mypage = identp(dummy)      ! my page zero #
c
        call exclusi ( mypage )     ! set me as exclusive
        call sharewi
        call hidewin ( mypage )
        call sharewi
        call quitwin
        stop
        end
