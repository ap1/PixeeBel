import re

def load( filename ):
   bins = {}

   binDescRe = re.compile( "(?P<name>\S+)\s+"
                           "(?P<freq_low>\d+)\s+"
                           "(?P<freq_high>\d+)\s+"
                           "(?P<threshold>\d+)" )

   with open( filename, "r" ) as f:
      lines = f.readlines()

   count = 0
   for line in [ l for l in lines if l[0] != "#" ]:
      m = binDescRe.match( line )
      if m:
         bins[ count ] = m.groupdict()
         count += 1

   return bins
