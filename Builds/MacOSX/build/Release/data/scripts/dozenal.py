def dozenaldigit(decimal):
   if decimal is 11:
      return 'E'
   elif decimal is 10:
      return 'X'
   else:
      return str(decimal)

def dozenal(decimal):
   ret = dozenaldigit(decimal % 12)
   decimal /= 12
   while (decimal > 0):
      ret = dozenaldigit(decimal % 12) + ret
      decimal /= 12
   return ret

for i in range(128):
   globals()['d'+dozenal(i)] = i 