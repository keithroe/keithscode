
w = [1,2]
z = [3,4]
a = [(3,4)]

[x*y for x in w for y in z ]
[x for x in w if x < 2 if x > 1 ] 
[(x,y) for x,y in a ] 
[x for x in z if x < 4 for y in z ] 

