/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

struct ngx_module_s {
	ngx_uint_t			ctx_index;	/* 当前模块在同类模块中的序号 */
	ngx_uint_t			index;		/* 当前模块在ngx_modules数组中的序号 */

	ngx_uint_t			spare0;	   /* 保留 */
	ngx_uint_t			spare1;	   /* 保留 */
	ngx_uint_t			spare2;	   /* 保留 */
	ngx_uint_t			spare3;	   /* 保留 */

	ngx_uint_t			version;	  /* 模块版本，目前为1 */

	void				 *ctx;		  /* 指向特定类型模块的公共接口 */
	ngx_command_t		*commands;	 /* 用于处理配置文件nginx.conf中的配置项 */
	ngx_uint_t			type;		 /* 当前模块类型 */

	/* 以下7个函数指针表示7个执行点，这些执行点将在Nginx启动和退出过程中被调用
	 * 如果不需要则设置为NULL
	 */
	ngx_int_t		   (*init_master)(ngx_log_t *log);	 /* 从未被调用，设为NULL */

	ngx_int_t		   (*init_module)(ngx_cycle_t *cycle); /* 启动worker子进程前调用 */

	ngx_int_t		   (*init_process)(ngx_cycle_t *cycle);/* 启动worker子进程后调用 */
	ngx_int_t		   (*init_thread)(ngx_cycle_t *cycle); /* 从未被调用，设为NULL */
	void				(*exit_thread)(ngx_cycle_t *cycle); /* 从未被调用，设为NULL */
	void				(*exit_process)(ngx_cycle_t *cycle);/* worker子进程推出前调用 */

	void				(*exit_master)(ngx_cycle_t *cycle); /* master进程退出前调用 */

	/* 以下全为保留字段 */
	uintptr_t			 spare_hook0;
	uintptr_t			 spare_hook1;
	uintptr_t			 spare_hook2;
	uintptr_t			 spare_hook3;
	uintptr_t			 spare_hook4;
	uintptr_t			 spare_hook5;
	uintptr_t			 spare_hook6;
	uintptr_t			 spare_hook7;
};

typedef struct {
	ngx_int_t   (*preconfiguration)(ngx_conf_t *cf);	// 解析配置文件前调用
	ngx_int_t   (*postconfiguration)(ngx_conf_t *cf);   // 解析完配置文件后调用

	void	   *(*create_main_conf)(ngx_conf_t *cf);	// 创建存储直属于http{}的配置项的结构体
	char	   *(*init_main_conf)(ngx_conf_t *cf, void *conf);  // 初始化main级别配置项

	void	   *(*create_srv_conf)(ngx_conf_t *cf);	 // 创建存储直属于srv{}的配置项的结构体
	char	   *(*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);  // 合并main级别和srv级别的同名配置项

	void	   *(*create_loc_conf)(ngx_conf_t *cf);	 // 创建存储直属于loc{}的配置项的结构体
	char	   *(*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);  // 合并srv级别和loc级别的同名配置项
} ngx_http_module_t;

truct ngx_command_s {
	ngx_str_t			 name;	 // 配置项名称
	ngx_uint_t			type;	 // 配置项类型，包括该配置项可以出现的位置和可以携带参数的个数

	// 出现name配置项后，调用此方法解析配置项参数
	char			   *(*set)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
	ngx_uint_t			conf;	 // 配置文件中的偏移量，确定将该配置项放入哪个存储结构体中
	ngx_uint_t			offset;   // 将该配置项放在存储结构体的哪个字段
	void				 *post;	 // 配置项读取后的处理方法
};

static ngx_command_t ngx_http_mytest_commands[] = {
	{
		ngx_string("mytest"),
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
		ngx_http_mytest,		// 出现mytest配置项时，ngx_http_mytest函数被调用
		NGX_HTTP_LOC_CONF_OFFSET,
		0,
		NULL,
	},
	ngx_null_command			// 以一个空的ngx_command_t作为结尾
};

static char* ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;

	// 找到mytest配置项所属的配置块
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	// 设置处理请求的方法，HTTP框架在处理用户请求进行到NGX_HTTP_CONTENT_PHASE阶段时
	// 如果主机域名、URI和mytest模块所在配置块名称相同，就会调用函数ngx_http_mytest_handler
	clcf->handler = ngx_http_mytest_handler;

	return NGX_CONF_OK;
}

static ngx_http_module_t ngx_http_mytest_module_ctx = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

ngx_module_t ngx_http_mytest_module = {
	NGX_MODULE_V1,	  // 0,0,0,0,0,0,1
	&ngx_http_mytest_module_ctx,
	ngx_http_mytest_commands,
	NGX_HTTP_MODULE,	// 定义模块类型

	/* Nginx在启动和退出时会调用下面7个回调方法 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING,  // 0,0,0,0,0,0,0,0,保留字段
};

static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r)
{
	// 请求的方法必须为GET或者HEAD
	if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD)))
		return NGX_HTTP_NOT_ALLOWED;

	// 丢弃请求中的包体
	ngx_int_t rc = ngx_http_discard_request_body(r);
	if (rc != NGX_OK)
		return rc;

	ngx_str_t type = ngx_string("text/plain");
	ngx_str_t response = ngx_string("Hello World!");	// 包体内容

	// 设置响应的HTTP头部
	r->headers_out.status = NGX_HTTP_OK;		   // 返回的响应码
	r->headers_out.content_length_n = response.len;	// 响应包体长度
	r->headers_out.content_type = type;				// Content-Type

	rc = ngx_http_send_header(r); // 发送HTTP头部
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
		return rc;

	// 如果响应不包含包体，则在此处可以直接返回rc

	// 分配响应包体空间，因为是异步发送，所以不要从栈中获得空间
	ngx_buf_t *b = ngx_create_temp_buf(r->pool, response.len);

	if (b == NULL)
		return NGX_HTTP_INTERNAL_SERVER_ERROR;

	ngx_memcpy(b->pos, response.data, response.len);
	b->last = b->pos + response.len;  // 指向数据末尾
	b->last_buf = 1;					// 声明这是最后一块缓冲区

	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;

	return ngx_http_output_filter(r, &out);   // 向用户发送响应包
}
