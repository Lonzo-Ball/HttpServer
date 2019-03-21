## 项目名称:
-
	基于HTTP协议的WEB服务器
## 项目功能：
-
	实现局域网内对HTTP请求的响应：
## 开发技术：
-
	C++、Linux、线程池技术、
## 项目流程：
-
### 一：创建一个服务器<br>
	程序启动，先构建一个 HttpdServer 对象。该对象的数据成员有三个：服务器端口号 port、监听描述符 listen_sock、指向线程池 ThreadPool对象的指针tp；<br>
核心方法有两个：用于初始化服务器的 InitServer() 函数和用于启动服务器的 Start() 函数。<br>
### 二：初始化服务器<br>
	HttpdServer 对象中的 InitServer() 函数用于初始化服务器。首先创建监听套接字，绑定服务器的地址信息，然后调用 listen() 系统调用使服务器处于监听状态；接着构建一个 ThreadPool 对象，并对其进行初始化。<br>
	线程池 ThreadPool 对象数据成员有六个，分别是：线程总数、处于休眠状态的线程数、任务队列 task_queue、互斥锁、互斥变量和一个用于标记线程是否退出的变量 is_quit;其中的一个方法是 InitThreadPool(),在初始化服务器阶段创建一定数量（本项目中是5个）的线程。<br>
### 三：启动服务器<br>
	在 Start() 函数内服务器不断的接受新的连接请求，并对请求进行处理。<br>
	当服务器接收到一个新的连接请求时，将该连接封装成任务，再将这个任务 push 到任务队列中，接着唤醒一个线程调用 Entry 类中的 HandlerRequest() 方法去处理这个任务。<br>
	执行 HandlerRequest 函数时：<br>
	先对 http 请求进行读取并解析：<br>
	1.读取请求行，并对其进行解析：http 请求的每一行以 \n \r 或 \r\n 结束的这一特点读取到请求行 ，利用请求行本身的格式特点，使用 stringstream 按照空格将请求方法、url、版本信息获取到。<br>
	(a) 判断请求方法是否合法：POST 和 GET 方法。这里做个简单处理，当方法为 post 方法是标记为 cgi 处理方式。<br>
	(b) 解析 url :当方法为 post 方法时，url 就是“请求资源的路径”；当方法为 get 方法时，需要判断 url 中有无 '?'，当没有 '?' 时 url 就是请求资源的路径；当有 '?' 时，'？' 之前的部分是请求资源的路径，'?' 之后的部分是请求参数，此时标记为 cgi 处理方式。<br>
	获取到路径之后，需要判断路径是否合法，是否合法的依据是有无这个路径下的文件。如果请求路径合法，还需判断请求资源是否为可执行程序，如果是，标记为cgi处理方式。<br>
	2.读取请求报头，并对其进行解析：以空行（\n）作为结束标志读取整个请求报头，然后构造键值对，键值对存放在 unordered_map 中。<br>
	3.读取空行<br>
	4.读取请求正文（如果有正文）：当方法为 post 时，才有必要读取请求正文。<br>
	-
	至此，http 请求的读取工作已经全部完成...<br>
	-
	构建 http 响应：<br>
	根据 cgi 的标记值，http 响应分为非 cgi 和 cgi 两种<br>
	1.构建非 cgi 响应<br>
	2.构建 cgi 响应<br>



















































	
