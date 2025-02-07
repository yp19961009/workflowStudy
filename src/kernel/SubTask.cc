/*
  Copyright (c) 2019 Sogou, Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Author: Xie Han (xiehan@sogou-inc.com)
*/

#include "SubTask.h"

void SubTask::subtask_done()
{
	SubTask *cur = this;
	ParallelTask *parent;
	SubTask **entry;

	while (1)
	{
		parent = cur->parent;
		entry = cur->entry;
		cur = cur->done();//这里不是存虚函数吗？没有实现怎么能直接调用
		if (cur)//指针为NULL的时候，相当于0，即false
		{
			cur->parent = parent;
			cur->entry = entry;
			if (parent)
				*entry = cur;

			cur->dispatch();
		}
		else if (parent)
		{
			if (__sync_sub_and_fetch(&parent->nleft, 1) == 0)
			{
				cur = parent;
				continue;
			}
		}

		break;
	}
}

void ParallelTask::dispatch()
{
	SubTask **end = this->subtasks + this->subtasks_nr;
	SubTask **p = this->subtasks;

	this->nleft = this->subtasks_nr;
	if (this->nleft != 0)
	{
		do
		{
			(*p)->parent = this;
			(*p)->entry = p;
			(*p)->dispatch();
		} while (++p != end);
	}
	else
		this->subtask_done();
}

