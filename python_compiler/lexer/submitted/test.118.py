 def perm(l):                       # error: first line indented
for i in range(len(l)):             # error: not indented
    s = l[:i] + l[i+1:]
        p = perm(l[:i] + l[i+1:])   # error: unexpected indent
        for x in p:
                r.append(l[i:i+1] + x)
            return r                # error: inconsistent dedent
3.14    10.    .001    1e100    3.14e-10    0e0
def perm(l):
        # Compute the list of all permutations of l
    if len(l) <= 1:
                  return [l]
    r = []
    for i in range(len(l)):
             s = l[:i] + l[i+1:]
             p = perm(s)
             for x in p:
              r.append(l[i:i+1] + x)
    return r
3.14j   10.j    10j     .001j   1e100j  3.14e-10j
7     2147483647                        0o177    0b100110111
3     79228162514264337593543950336     0o377    0x100000000
      79228162514264337593543950336              0xdeadbeef
month_names = ['Januari', 'Februari', 'Maart',      # These are the
               'April',   'Mei',      'Juni',       # Dutch names
               'Juli',    'Augustus', 'September',  # for the months
               'Oktober', 'November', 'December']   # of the year
# Comment 1
 # Comment 2

# Factorial:

def fact(  x\
):

  if x == -1:
    return 1.j

  elif x ==0:

    return 1
  else:

        return x* fact(x

- 1)

s = "foo\
\\ \n\'\""


fact(20)
