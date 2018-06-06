t = {}
t["foo"] = "bar"
h = Http2.req("www.google.fi", t, true)
t2 = Http2.headers(t)
print("--")
print(t2["foo"])
print(t2["bar"])
print("--")

t = Http2.datablk(h)
while true do
  print(t.data)
  if t.last then
    break
  end
  t = Http2.datablk(h)
end

Http2.free(h)

function f()
   local t
   local h
   local t2
   t = {}
   t["foo"] = "bar"
   h = Http2.req("www.google.fi", t, false)
   t2 = Http2.headers(t)
   if t2["foo"] ~= "bar" then
     print("Error")
   end
   if t2["bar"] ~= "foo" then
     print("Error")
   end
   
   t = Http2.datablk(h)
   while true do
     if t.last then
       break
     end
     t = Http2.datablk(h)
   end
   Http2.free(h)
end

function coro2()
   local t
   local h
   local t2
   t = {}
   t["foo"] = "bar"
   h = Http2.req("www.google.fi", t, false)
   coroutine.yield(1)
   t2 = Http2.headers(t)
   if t2["foo"] ~= "bar" then
     print("Error")
   end
   if t2["bar"] ~= "foo" then
     print("Error")
   end

   coroutine.yield(2)
   
   t = Http2.datablk(h)
   while true do
     if t.last then
       break
     end
     coroutine.yield(3)
     t = Http2.datablk(h)
   end
   Http2.free(h)
   return -1
end

function coro()
   return coro2()
end
