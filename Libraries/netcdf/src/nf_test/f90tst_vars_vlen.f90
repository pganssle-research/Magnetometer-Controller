! This is part of the netCDF package.
! Copyright 2009 University Corporation for Atmospheric Research/Unidata.
! See COPYRIGHT file for conditions of use.

! This program tests netCDF-4 vlen variable functions from fortran 90.

! $Id: f90tst_vars_vlen.f90,v 1.3 2009/02/02 19:31:25 ed Exp $

program f90tst_vars_vlen
  use typeSizes
  use netcdf
  implicit none
!  include 'netcdf.inc'

!   ! This is the name of the data file we will create.
  character(len=*) :: FILE_NAME
  parameter (FILE_NAME='f90tst_vars_vlen.nc')

!   ! NetCDF IDs.
   integer :: ncid, vlen_typeid

   integer :: max_types
   parameter (max_types = 1)

!   ! Need these to read type information.
   integer :: num_types
   integer, dimension(max_types) :: typeids
   integer :: base_type, base_size
   character(len=80) :: type_name
   integer :: type_size, nfields, class

!   ! Information for the vlen type we will define.
   character(len=*) vlen_type_name
   parameter (vlen_type_name = 'vlen_type')

!   ! Some data about and for the vlen.
   integer :: vlen_len, vlen_len_in
   parameter (vlen_len = 5)
   integer, dimension(vlen_len) :: data1, data1_in

!   ! These must be big enough to hold the stuct nc_vlen in netcdf.h.
   integer, dimension(10) :: vlen, vlen_in

!   ! Loop indexes, and error handling.
   integer :: x, retval

   print *, ''
   print *,'*** Testing VLEN types.'

   do x = 1, vlen_len
      data1(x) = x
   end do

!   ! Create the netCDF file.
   call check(nf90_create(FILE_NAME, NF90_NETCDF4, ncid))

!   ! Create the vlen type.
   call check(nf90_def_vlen(ncid, vlen_type_name, nf90_int, vlen_typeid))

!   ! Set up the vlen with this helper function, since F77 can't deal
!   ! with pointers.
   call check(nf90_put_vlen_element(ncid, vlen_typeid, vlen, vlen_len, data1))

!   ! Write the vlen attribute.
   call check(nf90_put_att(ncid, NF90_GLOBAL, 'att1', vlen_typeid, 1, vlen))

!   ! Close the file. 
   call check(nf90_close(ncid))

!   ! Reopen the file.
   call check(nf90_open(FILE_NAME, NF90_NOWRITE, ncid))

!   ! Get the typeids of all user defined types.
   call check(nf90_inq_typeids(ncid, num_types, typeids))
   if (num_types .ne. max_types) goto 99

!   ! Use nf_inq_user_type to confirm this is an vlen type, with base
!   ! size 4, base type NF90_INT.
   call check(nf90_inq_user_type(ncid, typeids(1), type_name, type_size, &
        base_type, nfields, class))
   if (type_name(1:len(vlen_type_name)) .ne. vlen_type_name .or. &
         type_size .ne. 4 .or. base_type .ne. nf90_int .or. &
         nfields .ne. 0 .or. class .ne. nf90_vlen) goto 99

!   ! Use nf_inq_vlen and make sure we get the same answers as we did
!   ! with nf_inq_user_type.
   call check(nf90_inq_vlen(ncid, typeids(1), type_name, base_size, base_type))
   if (type_name(1:len(vlen_type_name)) .ne. vlen_type_name .or. &
        base_type .ne. nf90_int .or. base_size .ne. 4) goto 99

!   ! Read the vlen attribute.
   call check(nf90_get_att(ncid, NF90_GLOBAL, 'att1', vlen_in))

!   ! Get the data from the vlen we just read.
   call check(nf90_get_vlen_element(ncid, vlen_typeid, vlen_in, &
        vlen_len_in, data1_in))
   if (vlen_len_in .ne. vlen_len) goto 99

!   ! Check the data
   do x = 1, vlen_len
      if (data1(x) .ne. data1_in(x)) goto 99
   end do

!   ! Close the file. 
   call check(nf90_close(ncid))

  print *,'*** SUCCESS!'
99 continue
  print *,'*** VLENS NOT IMPLEMENTED YET!'
contains
!     This subroutine handles errors by printing an error message and
!     exiting with a non-zero status.
  subroutine check(errcode)
    use netcdf
    implicit none
    integer, intent(in) :: errcode
    
    if(errcode /= nf90_noerr) then
       print *, 'Error: ', trim(nf90_strerror(errcode))
       stop 2
    endif
  end subroutine check
end program f90tst_vars_vlen
