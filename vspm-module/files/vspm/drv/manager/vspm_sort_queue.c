/*************************************************************************/ /*
 * VSPM
 *
 * Copyright (C) 2015-2017 Renesas Electronics Corporation
 *
 * License        Dual MIT/GPLv2
 *
 * The contents of this file are subject to the MIT license as set out below.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU General Public License Version 2 ("GPL") in which case the provisions
 * of GPL are applicable instead of those above.
 *
 * If you wish to allow use of your version of this file only under the terms of
 * GPL, and not to allow others to use your version of this file under the terms
 * of the MIT license, indicate your decision by deleting the provisions above
 * and replace them with the notice and other provisions required by GPL as set
 * out in the file called "GPL-COPYING" included in this distribution. If you do
 * not delete the provisions above, a recipient may use your version of this
 * file under the terms of either the MIT license or GPL.
 *
 * This License is also included in this distribution in the file called
 * "MIT-COPYING".
 *
 * EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * GPLv2:
 * If you wish to use this file under the terms of GPL, following terms are
 * effective.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */ /*************************************************************************/

#include <linux/string.h>

#include "frame.h"

#include "vspm_public.h"
#include "vspm_ip_ctrl.h"
#include "vspm_main.h"
#include "vspm_log.h"

#include "vspm_lib_public.h"
#include "vspm_common.h"


/******************************************************************************
 * Function:		vspm_inc_sort_queue_initialize
 * Description:	Initialize the queue information.
 * Returns:		R_VSPM_OK
 ******************************************************************************/
long vspm_inc_sort_queue_initialize(struct vspm_queue_info *queue_info)
{
	queue_info->data_count = 0;
	queue_info->first_job_info = NULL;

	return R_VSPM_OK;
}


/******************************************************************************
 * Function:		vspm_inc_sort_queue_entry
 * Description:	Add a job information to the queue.
 * Returns:		R_VSPM_OK/R_VSPM_NG
 ******************************************************************************/
long vspm_inc_sort_queue_entry(
	struct vspm_queue_info *queue_info, struct vspm_job_info *job_info)
{
	struct vspm_job_info **work_job_info;
	unsigned short i;

	/* check data counter */
	if (queue_info->data_count >= VSPM_MAX_ELEMENTS) {
		EPRINT("%s queue full\n", __func__);
		return R_VSPM_NG;
	}

	/* entry queue */
	work_job_info = (struct vspm_job_info **)&queue_info->first_job_info;
	for (i = 0; i < queue_info->data_count; i++) {
		if (job_info->entry.job_priority >
			(*work_job_info)->entry.job_priority) {
			job_info->next_job_info = (void *)(*work_job_info);
			break;
		}
		work_job_info = (struct vspm_job_info **)
			&(*work_job_info)->next_job_info;
	}

	*work_job_info = job_info;

	/* increment data counter */
	queue_info->data_count++;

	return R_VSPM_OK;
}


/******************************************************************************
 * Function:		vspm_inc_sort_queue_refer
 * Description:	Get a job information from queue.
 * Returns:		R_VSPM_OK/R_VSPM_NG
 ******************************************************************************/
long vspm_inc_sort_queue_refer(struct vspm_queue_info *queue_info,
	unsigned short index, struct vspm_job_info **p_job_info)
{
	struct vspm_job_info *work_job_info;
	unsigned short i;

	/* check data counter */
	if (index >= queue_info->data_count) {
		EPRINT("%s out of index %d data_count=%d\n",
			__func__, index, queue_info->data_count);
		return R_VSPM_NG;
	}

	work_job_info = (struct vspm_job_info *)queue_info->first_job_info;
	for (i = 0; i < index; i++) {
		work_job_info =
			(struct vspm_job_info *)work_job_info->next_job_info;
	}

	*p_job_info = work_job_info;

	return R_VSPM_OK;
}


/******************************************************************************
 * Function:		vspm_inc_sort_queue_remove
 * Description:	Remove a job information from queue.
 * Returns:		R_VSPM_OK/R_VSPM_NG
 ******************************************************************************/
long vspm_inc_sort_queue_remove(
	struct vspm_queue_info *queue_info, unsigned short index)
{
	struct vspm_job_info **work_job_info;
	unsigned short i;

	/* check data counter */
	if (index >= queue_info->data_count) {
		EPRINT("%s out of index %d data_count=%d\n",
			__func__, index, queue_info->data_count);
		return R_VSPM_NG;
	}

	work_job_info = (struct vspm_job_info **)&queue_info->first_job_info;
	for (i = 0; i < index; i++) {
		work_job_info = (struct vspm_job_info **)
			&(*work_job_info)->next_job_info;
	}

	*work_job_info =
		(struct vspm_job_info *)(*work_job_info)->next_job_info;

	/* decrement data counter */
	queue_info->data_count--;

	return R_VSPM_OK;
}


/******************************************************************************
 * Function:		vspm_inc_sort_queue_get_count
 * Description:	Get the number of entry.
 * Returns:		number of entry
 ******************************************************************************/
unsigned short vspm_inc_sort_queue_get_count(struct vspm_queue_info *queue_info)
{
	return queue_info->data_count;
}


/******************************************************************************
 * Function:		vspm_inc_sort_queue_find_item
 * Description:	Search a job information from queue.
 * Returns:		R_VSPM_OK/R_VSPM_NG
 ******************************************************************************/
long vspm_inc_sort_queue_find_item(struct vspm_queue_info *queue_info,
	struct vspm_job_info *job_info, unsigned short *p_index)
{
	struct vspm_job_info *work_job_info;
	unsigned short i;

	work_job_info = (struct vspm_job_info *)queue_info->first_job_info;
	for (i = 0; i < queue_info->data_count; i++) {
		if (job_info == work_job_info)
			break;

		work_job_info =
			(struct vspm_job_info *)work_job_info->next_job_info;
	}

	if (i >= queue_info->data_count) {
		EPRINT("%s not found job_info, data_count=%d\n",
			__func__, queue_info->data_count);
		return R_VSPM_NG;
	}

	*p_index = i;
	return R_VSPM_OK;
}

