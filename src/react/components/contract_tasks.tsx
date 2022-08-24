/* eslint-disable @typescript-eslint/no-explicit-any */
import React, { useState } from 'react';
import play_img from '../assets/play.png';
import pause_img from '../assets/pause.png';
import view_img from '../assets/view.png';
import delete_img from '../assets/delete.png';

type show = 'none' | 'block'

interface ContractTaskEntry {
    task_id: number;
    blockchain: string;
    contract_address: string;
    wallet_group_name: string;
    time_started: number;
}

class ContractTasks {
    private static display: show | undefined;
    public static update_display: React.Dispatch<React.SetStateAction<show | undefined>>;
    private static contract_tasks: React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined;
    private static update_contract_tasks: React.Dispatch<React.SetStateAction<React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined>>;
    public static tsx_wrap_contract_tasks = (array: ContractTaskEntry[]): void => {
        const react_array: React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] = [];
        array.forEach((contract_task) => {
            react_array.push(
                <tr>
                    <td className='text-sm text-white regular'>{ contract_task.task_id }</td>
                    <td className='text-sm text-white regular'>{ contract_task.blockchain }</td>
                    <td className='text-sm text-white regular'><span className='add-underline' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['href', 'main', { blockchain: contract_task.blockchain, contract: contract_task.contract_address }]) } }>{ contract_task.contract_address }</span></td>
                    <td className='text-sm text-white regular'>{ contract_task.wallet_group_name }</td>
                    <td className='text-sm text-white regular'>{ new Date(contract_task.time_started).toISOString() }</td>
                    <td className='text-sm text-white regular icon-hover' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['play', 'main', { task_type: 'contract_task', task_id: contract_task.task_id }]) } }>
                        <img className='table-icon play-btn unselectable' src={ play_img } alt=''/>
                    </td>
                    <td className='text-sm text-white regular icon-hover' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['pause', 'main', { task_type: 'contract_task', task_id: contract_task.task_id }]) } }>
                        <img className='table-icon play-btn unselectable' src={ pause_img } alt=''/>
                    </td>
                    <td className='text-sm text-white regular icon-hover' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['open', 'view_contract_task', { task_id: contract_task.task_id }]) } }>
                        <img className='table-icon play-btn unselectable' src={ view_img } alt=''/>
                    </td>
                    <td className='text-sm text-white regular icon-hover' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['open', 'confirmation', { confirm_type: 'contract_task', task_id: contract_task.task_id }]) } }>
                        <img className='table-icon play-btn unselectable' src={ delete_img } alt=''/>
                    </td>
                </tr>
            );
        });
        this.update_contract_tasks(react_array);
    };
    public static Make: React.FC = () => {
        [this.display, this.update_display] = useState<show | undefined>('none');
        [this.contract_tasks, this.update_contract_tasks] = useState<React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined>();
        return (
            <div className='other-pages' style={{ display: `${ this.display }` }}>
                <div className='d-flex flex-row align-items-center justify-content-between w-100 mb-4 unselectable'>
                <div className='placement text-lg bold text-white ms-3 mb-0 unselectable'>Manage Contract Tasks</div>
                    <div className='d-flex flex-row align-items-center unselectable'>
                        <div className='transparent-btn text-sm text-white d-flex flex-row align-items-center me-3 unselectable' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['open', 'create_contract_task', null]) } }>Create Contract Task</div>
                    </div>
                </div>
                <div className='container long-table special-scroll-bar unselectable'>
                    <table className='table table-head unselectable'>
                        <thead className='thead-dark unselectable'>
                            <tr>
                                <th className='text-md text-white regular unselectable'>ID</th>
                                <th className='text-md text-white regular unselectable'>Blockchain</th>
                                <th className='text-md text-white regular unselectable'>Contract</th>
                                <th className='text-md text-white regular unselectable'>Wallet Group</th>
                                <th className='text-md text-white regular unselectable'>Time Started</th>
                                <th/>
                                <th/>
                                <th/>
                                <th/>
                            </tr>
                        </thead>
                        <tbody className='unselectable'>{ this.contract_tasks }</tbody>
                    </table>
                </div>
            </div>
        );
    };
}

export { ContractTasks }