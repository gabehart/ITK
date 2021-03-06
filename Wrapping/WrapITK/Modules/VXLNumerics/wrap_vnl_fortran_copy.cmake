# auto include feature must be disable because the class is not in the file
# with the same name
set(WRAPPER_AUTO_INCLUDE_HEADERS OFF)
WRAP_INCLUDE("vnl/vnl_fortran_copy.h")

WRAP_CLASS("vnl_fortran_copy")
  WRAP_TEMPLATE("${ITKM_D}" "${ITKT_D}")
  WRAP_TEMPLATE("_vcl_complex${ITKM_D}" "vcl_complex<${ITKT_D}>")
  WRAP_TEMPLATE("${ITKM_F}" "${ITKT_F}")
  WRAP_TEMPLATE("_vcl_complex${ITKM_F}" "vcl_complex<${ITKT_F}>")
END_WRAP_CLASS()
