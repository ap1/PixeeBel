
def toUbyte( c ):
   '''Convert signed byte to unsigned byte.'''
   if c < -128:
      # this should be 128 because we return 
      # to unsigned byte
      return 128

   if c < 0:
      return 256 + c

   if c > 127:
      return 127

   return c

def writeListAsRaw( data_in, filename="data.raw" ):
   '''Write a list of integers to filename.'''
   with open( filename, "wb" ) as f:
      data = bytearray( data_in )
      f.write( data )

