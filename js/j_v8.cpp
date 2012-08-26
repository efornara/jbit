/*
 * Copyright (C) 2012  Emanuele Fornara
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

// j_v8.cpp

extern "C" {
#include "defs.h"
}

#include <string.h>
#include <assert.h>

#include <v8.h>

using namespace v8;

static Handle<Value> update(const Arguments& args) {
	if (args.Length() != 1)
		return ThrowException(String::New("usage: c__update(video)"));
	Handle<Array> array = args[0].As<Array>();
	if (array->Length() != 40)
		return ThrowException(String::New("video: expected int[40]"));
	unsigned char video[40];
	for (int i = 0; i < 40; i++)
		video[i] = (unsigned char)array->Get(i)->Int32Value();
	const char *err = c_update(video);
	if (err != NULL)
		return ThrowException(String::New(err));
	return Undefined();
}

static Handle<Value> setstatus(const Arguments& args) {
	if (args.Length() != 1)
		return ThrowException(String::New("usage: c__setstatus(status)"));
	String::AsciiValue ascii(args[0]);
	const char *err = c_setstatus(*ascii);
	if (err != NULL)
		return ThrowException(String::New(err));
	return Undefined();
}

static Persistent<Context> create_context() {
	Handle<ObjectTemplate> global = ObjectTemplate::New();
	global->Set(String::New("c__update"), FunctionTemplate::New(update));
	global->Set(String::New("c__setstatus"), FunctionTemplate::New(setstatus));
	return Context::New(NULL, global);
}

static Persistent<Context> context;
static Persistent<Script> do_advance;

void jsengine_init(void)
{
	HandleScope handle_scope;
	context = create_context();
	assert(!context.IsEmpty());
    context->Enter();
}

void jsengine_cleanup(void)
{
	HandleScope handle_scope;
	do_advance.Dispose();
    context->Exit();
	context.Dispose();
}

void jsengine_load(const char *expr)
{
	HandleScope handle_scope;
	Context::Scope context_scope(context);
	Handle<String> source = String::New(expr);
	Handle<Script> script = Script::Compile(source);
	script->Run();
}

int jsengine_run(const char *expr)
{
	HandleScope handle_scope;
	Context::Scope context_scope(context);
	Handle<Value> result;
	if (!strcmp("js__advance();", expr)) {
		if (do_advance.IsEmpty()) {
			Handle<String> source = String::New(expr);
			Handle<Script> script = Script::Compile(source);
			do_advance = Persistent<Script>::New(script);
		}
		result = do_advance->Run();
	} else {
		Handle<String> source = String::New(expr);
		Handle<Script> script = Script::Compile(source);
		result = script->Run();
	}
	return result->Int32Value();
}
