if 1900 < year < 2100 and 1 <= month <= 12 \
   and 1 <= day <= 31 and 0 <= hour < 24 \
   and 0 <= minute < 60 and 0 <= second < 60:   # Looks like a valid date
        return 1

a = \ # Comments after explicit line feeds are no-nos
 b

\
\
\
\

 # don't allow whitespace after \
d = \  
    e

s = "This is the #comment\
 string"

month_names = ['Januari', 'Februari', 'Maart',      # These are the
               'April',   'Mei',      'Juni',       # Dutch names
                 'Juli',    'Augustus', 'September',  # for the months
             'Oktober', 'November', 'December']   # of the year

def fact(  x, y\
): # this function rocks

  # blah blah
  if x == -1:
    return 1.j

  elif x ==0:

    return 1
  else:

        return x* fact(x

   - \
   1 \
 )

s = "foo\
\\ \n\'\""
