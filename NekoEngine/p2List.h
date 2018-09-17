#ifndef __p2List_H__
#define __p2List_H__

#include "Globals.h"

/**
* Contains items from double linked list
*/
template<class tdata>
struct p2List_item
{
	tdata                 data;
	p2List_item<tdata>*   next;
	p2List_item<tdata>*   prev;

	inline p2List_item(const tdata& _data)
	{
		data = _data;
		next = prev = NULL;
	}

	~p2List_item()
	{}
};

/**
* Manages a double linked list
*/
template<class tdata>
class p2List
{

private:

	p2List_item<tdata>*   start;
	p2List_item<tdata>*   end;
	unsigned int  size;

public:

	/**
	* Constructor
	*/
	inline p2List()
	{
		start = end = NULL;
		size = 0;
	}

	/**
	* Destructor
	*/
	~p2List()
	{
		clear();
	}

	p2List_item<tdata>* getFirst() const
	{
		return start;
	}

	p2List_item<tdata>* getLast() const
	{
		return end;
	}

	/**
	* Get Size
	*/
	unsigned int count() const
	{
		return size;
	}

	/**
	* Add new item
	*/
	p2List_item<tdata>* add(const tdata& item)
	{
		p2List_item<tdata>*   p_data_item;
		p_data_item = new p2List_item < tdata >(item);

		if(start == NULL)
		{
			start = end = p_data_item;
		}
		else
		{
			p_data_item->prev = end;
			end->next = p_data_item;
			end = p_data_item;
		}
		++size;
		return(p_data_item);
	}

	/**
	* Find by index
	*/
	bool at(unsigned int index, tdata& data) const
	{
		bool ret = false;
		unsigned int i = 0;
		p2List_item<tdata>*   p_data = start;

		for(unsigned int i = 0; i < index && p_data != NULL; ++i)
			p_data = p_data->next;

		if(p_data != NULL)
		{
			ret = true;
			data = p_data->data;
		}

		return ret;
	}

	/**
	* Deletes an item from the list
	*/
	bool del(p2List_item<tdata>* item)
	{
		if(item == NULL)
		{
			return (false);
		}

		// Now reconstruct the list
		if(item->prev != NULL)
		{
			item->prev->next = item->next;

			if(item->next != NULL)
			{
				item->next->prev = item->prev;
			}
			else
			{
				end = item->prev;
			}
		}
		else
		{
			if(item->next)
			{
				item->next->prev = NULL;
				start = item->next;
			}
			else
			{
				start = end = NULL;
			}
		}

		delete item;
		--size;
		return(true);
	}

	/**
	* Destroy and free all mem
	*/
	void clear()
	{
		p2List_item<tdata>*   p_data;
		p2List_item<tdata>*   p_next;
		p_data = start;

		while(p_data != NULL)
		{
			p_next = p_data->next;
			delete (p_data);
			p_data = p_next;
		}

		start = end = NULL;
		size = 0;
	}

	/**
	* returns the first apperance of data as index (-1 if not found)
	*/
	int find(const tdata& data)
	{
		p2List_item<tdata>* tmp = start;
		int index = 0;

		while(tmp != NULL)
		{
			if(tmp->data == data)
				return(index);

			++index;
			tmp = tmp->next;
		}
		return (-1);
	}


	/**
	* returns the first apperance of data as index (-1 if not found)
	*/
	p2List_item<tdata>* findNode(const tdata& data)
	{
		p2List_item<tdata>* tmp = start;

		while(tmp != NULL)
		{
			if(tmp->data == data)
				return(tmp);
			tmp = tmp->next;
		}

		return (NULL);
	}
};
#endif /*__p2List_H__*/