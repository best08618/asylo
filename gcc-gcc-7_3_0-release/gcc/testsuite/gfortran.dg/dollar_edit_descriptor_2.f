! { dg-do run }
! { dg-options "-w" }
! PR25545 internal file and dollar edit descriptor.
      program main
      character*20 line
      line = '1234567890ABCDEFGHIJ'
      write (line, '(A$)') 'asdf'
      if (line.ne.'asdf') call abort()
      end
