import numpy.fft as fft

def lpf( N, cutoffInHz ):
   filt = [ 1.0 for i in range( N ) ]
   freq = fft.fftfreq( N, d=0.000110 )

   for i, f in enumerate( freq ):
      if abs( f ) >= cutoffInHz:
         filt[i] = 0.0

   return filt

def hpf( N, cutoffInHz ):
   filt = [ 1.0 for i in range( N ) ]
   freq = fft.fftfreq( N, d=0.000110 )

   for i, f in enumerate( freq ):
      if abs( f ) < cutoffInHz:
         filt[i] = 0.0

   return filt

def mult( X, Y ):
   return [ x * y for x, y in zip( X, Y ) ]

def inv( filt ):
   def inv_( x ):
      return 0.0 if x else 1.0
   return [ inv_( f ) for f in filt ]

def apply( data, filt, freq=None ):
   '''Takes the time domain (or already available frequency representation)
   of a signal and applies the filter and returns the time-domain result
   and the frequency domain representation.'''

   freq = fft.fft( data ) if freq is None else freq
   freq = mult( freq, filt )
   ifft = fft.ifft( freq )

   return ifft, freq

