/* eslint-disable @typescript-eslint/no-explicit-any */
import React, { useState } from 'react';
import play_img from '../assets/play.png';
import pause_img from '../assets/pause.png';
import view_img from '../assets/view.png';
import delete_img from '../assets/delete.png';

type show = 'none' | 'block'

interface SniperTaskEntry {
    task_id: number;
    blockchain: string;
    marketplace: string;
    collection_slug: string;
    minimum_value: number;
    maximum_value: number;
}

class SniperTasks {
    private static display: show | undefined;
    public static update_display: React.Dispatch<React.SetStateAction<show | undefined>>;
    private static sniper_tasks: React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined;
    private static update_sniper_tasks: React.Dispatch<React.SetStateAction<React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined>>;
    public static tsx_wrap_sniper_tasks = (array: SniperTaskEntry[]): void => {
        const react_array: React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] = [];
        array.forEach((sniper_task: SniperTaskEntry) => {
            react_array.push(
                <tr>
                    <td className='text-sm text-white regular'>{ sniper_task.task_id }</td>
                    <td className='text-sm text-white regular'>{ sniper_task.blockchain }</td>
                    <td className='text-sm text-white regular'>{ sniper_task.marketplace }</td>
                    <td className='text-sm text-white regular'><span className='add-underline' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['href', 'main', { marketplace: sniper_task.marketplace, slug: sniper_task.collection_slug, }]) } }>{ sniper_task.collection_slug }</span></td>
                    <td className='text-sm text-white regular'>{ sniper_task.minimum_value }</td>
                    <td className='text-sm text-white regular'>{ sniper_task.maximum_value }</td>
                    <td className='text-sm text-white regular icon-hover' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['play', 'main', { task_type: 'sniper_task', task_id: sniper_task.task_id }]) } }>
                        <img className='table-icon play-btn unselectable' src={ play_img } alt=''/>
                    </td>
                    <td className='text-sm text-white regular icon-hover' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['pause', 'main', { task_type: 'sniper_task', task_id: sniper_task.task_id }]) } }>
                        <img className='table-icon play-btn unselectable' src={ pause_img } alt=''/>
                    </td>
                    <td className='text-sm text-white regular icon-hover' onClick={ ()=> { (window as any).electron.ipcRenderer.message('perform', ['open', 'view_sniper_task', { task_id: sniper_task.task_id }]) } }>
                        <img className='table-icon play-btn unselectable' src={ view_img } alt=''/>
                    </td>
                    <td className='text-sm text-white regular icon-hover' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['open', 'confirmation', { confirm_type: 'sniper_task', task_id: sniper_task.task_id }]) } }>
                        <img className='table-icon play-btn unselectable' src={ delete_img } alt=''/>
                    </td>
                </tr>
            );
        });
        this.update_sniper_tasks(react_array);
    };
    public static Make: React.FC = () => {
        [this.display, this.update_display] = useState<show | undefined>('none');
        [this.sniper_tasks, this.update_sniper_tasks] = useState<React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined>();
        return (
            <div className='other-pages' style={{ display: `${ this.display }` }}>
                <div className='d-flex flex-row align-items-center justify-content-between w-100 mb-4 unselectable'>
                    <div className='placement text-lg bold text-white ms-3 mb-0 unselectable'>Manage Sniper Tasks</div>
                    <div className='d-flex flex-row align-items-center unselectable'>
                        <div className='transparent-btn text-sm text-white d-flex flex-row align-items-center me-3 unselectable' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['open', 'create_sniper_task', null]) } }>Create Sniper Task</div>
                    </div>
                </div>
                <div className='container long-table special-scroll-bar unselectable'>
                    <table className='table table-head unselectable'>
                        <thead className='thead-dark unselectable'>
                            <tr>
                                <th className='text-md text-white regular unselectable'>ID</th>
                                <th className='text-md text-white regular unselectable'>Blockchain</th>
                                <th className='text-md text-white regular unselectable'>Marketplace</th>
                                <th className='text-md text-white regular unselectable'>Collection Slug</th>
                                <th className='text-md text-white regular unselectable'>Minimum Value</th>
                                <th className='text-md text-white regular unselectable'>Maximum Value</th>
                                <th/>
                                <th/>
                                <th/>
                                <th/>
                            </tr>
                        </thead>
                        <tbody className='unselectable'>{ this.sniper_tasks }</tbody>
                    </table>
                </div>
            </div>
        );
    };
}

export { SniperTasks }