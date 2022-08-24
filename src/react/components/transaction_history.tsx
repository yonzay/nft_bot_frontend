/* eslint-disable @typescript-eslint/no-explicit-any */
import React, { useState } from 'react';

type show = 'none' | 'block'

interface TransactionEntry {
    blockchain: string;
    transaction_hash: string;
    date_initiated: number;
}

class TransactionHistory {
    private static display: show | undefined;
    public static update_display: React.Dispatch<React.SetStateAction<show | undefined>>;
    private static transaction_history: React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined;
    private static update_transaction_history: React.Dispatch<React.SetStateAction<React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined>>;
    public static tsx_wrap_transaction_history = (array: TransactionEntry[]): void => {
        const react_array: React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] = [];
        array.forEach((transactions) => {
            react_array.push(
                <tr>
                    <td className='text-sm text-white regular'>{ transactions.blockchain }</td>
                    <td className='text-sm text-white regular'><span className='add-underline' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['href', 'main', { blockchain: transactions.blockchain, txn: transactions.transaction_hash }]) } }>{ transactions.transaction_hash }</span></td>
                    <td className='text-sm text-white regular'>{ new Date(transactions.date_initiated).toISOString() }</td>
                </tr>
            );
        });
        this.update_transaction_history(react_array);
    };
    public static tsx_insert_transaction_history = (txn: TransactionEntry): void => {
        this.transaction_history.push(
            <tr>
                <td className='text-sm text-white regular'>{ txn.blockchain }</td>
                <td className='text-sm text-white regular'><span className='add-underline' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['href', 'main', { blockchain: txn.blockchain, txn: txn.transaction_hash }]) } }>{ txn.transaction_hash }</span></td>
                <td className='text-sm text-white regular'>{ new Date(txn.date_initiated).toISOString() }</td>
            </tr>
        );
        this.update_transaction_history(this.transaction_history);
    };
    public static Make: React.FC = () => {
        [this.display, this.update_display] = useState<show | undefined>('none');
        [this.transaction_history, this.update_transaction_history] = useState<React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined>([]);
        return (
            <div className='other-pages' style={{ display: `${ this.display }` }}>
                <div className='d-flex flex-row align-items-center justify-content-between w-100 mb-4 unselectable'>
                    <div className='placement text-lg bold text-white ms-3 mb-0 unselectable'>Transaction History</div>
                </div>
                <div className='container long-table special-scroll-bar unselectable'>
                    <table className='table table-head'>
                        <thead className='thead-dark'>
                            <tr>
                                <th className='text-md text-white regular'>Blockchain</th>
                                <th className='text-md text-white regular'>Transaction Hash</th>
                                <th className='text-md text-white regular'>Date Initiated</th>
                            </tr>
                        </thead>
                        <tbody className='unselectable'>{ this.transaction_history }</tbody>
                    </table>
                </div>
            </div>
        );
    };
}

export { TransactionHistory }