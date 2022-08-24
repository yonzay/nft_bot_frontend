/* eslint-disable @typescript-eslint/no-explicit-any */
import React, { useState } from 'react';

type show = 'none' | 'block'

interface DashboardCard {
    nft_image_url: string | undefined;
    nft_iteration: number | undefined;
    welcome_message: string | undefined;
    username: string | undefined;
    quote: string | undefined;
}

interface Analytics {
    total_transactions: number;
    total_spent: number;
    spent_today: number;
}

interface TransactionEntry {
    blockchain: string;
    transaction_hash: string;
    date_initiated: number;
}

class Dashboard {
    private static display: show | undefined;
    public static update_display: React.Dispatch<React.SetStateAction<show | undefined>>;
    private static dashboard_card: DashboardCard | undefined;
    public static update_dashboard_card: React.Dispatch<React.SetStateAction<DashboardCard | undefined>>;
    private static analytics: Analytics | undefined;
    public static update_analytics: React.Dispatch<React.SetStateAction<Analytics | undefined>>;
    private static total_value_spent: number | undefined;
    public static update_total_value_spent: React.Dispatch<React.SetStateAction<number | string | undefined>>;
    private static todays_transactions: React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined;
    private static update_todays_transactions: React.Dispatch<React.SetStateAction<React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined>>;
    public static tsx_wrap_todays_transactions = (array: TransactionEntry[]): void => {
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
        this.update_todays_transactions(react_array);
    };
    public static Make: React.FC = () => {
        [this.display, this.update_display] = useState<show | undefined>('none');
        [this.dashboard_card, this.update_dashboard_card] = useState<DashboardCard | undefined>();
        [this.analytics, this.update_analytics] = useState<Analytics | undefined>();
        [this.total_value_spent, this.update_total_value_spent] = useState<number | undefined>(0);
        [this.todays_transactions, this.update_todays_transactions] = useState<React.DetailedHTMLProps<React.HTMLAttributes<HTMLTableRowElement>, HTMLTableRowElement>[] | undefined>();
        return (
            <>
                <div className='dashboard-card' style={{ display: `${ this.display }` }}>
                    <div className='d-flex flex-row align-items-center p-3 lighter-bg outer-round h-100 ps-2 unselectable'>
                        <img className='inner-round me-3 user-img unselectable' src={ this.dashboard_card?.nft_image_url } alt=''/>
                        <div className='d-flex flex-column align-items-start unselectable'>
                            <div className='d-flex flex-row align-items-center justify-content-center mb-2 unselectable'>
                                <div className='purple-line me-2 unselectable'/>
                                <p className='text-sm text-white regular mb-0 unselectable'>MoonDweller#{ this.dashboard_card?.nft_iteration }</p>
                            </div>
                            <h6 className='text-lg text-white bold unselectable'>{ this.dashboard_card?.welcome_message }, <span className='text-purple unselectable'>{ this.dashboard_card?.username }</span>.</h6>
                            <p className='text-xs text-white regular semi-bold unselectable'>{ this.dashboard_card?.quote }</p>
                        </div>
                    </div>
                </div>
                <div className='analytics' style={{ display: `${ this.display }` }}>
                    <div className='d-flex flex-column align-items-start p-3 lighter-bg outer-round h-100 unselectable'>
                        <div className='d-flex flex-row mb-2 align-items-center unselectable'>
                            <p className='text-sm text-white regular unselectable'>Analytics</p>
                        </div>
                        <div className='d-flex flex-row mb-1 align-items-center unselectable'>
                            <p className='text-sm bold text-purple me-1 semi-bold unselectable'>{ this.analytics?.total_transactions }</p>
                            <p className='text-xs text-white regular unselectable'>Total Transactions</p>
                        </div>
                        <div className='d-flex flex-row mb-1 align-items-center unselectable'>
                            <p className='text-sm bold text-purple me-1 semi-bold unselectable'><span className='text-white unselectable'>$</span>{ this.analytics?.total_spent }</p>
                            <p className='text-xs text-white regular unselectable'>Total Spent</p>
                        </div>
                        <div className='d-flex flex-row mb-1 align-items-center unselectable'>
                            <p className='text-sm bold text-purple me-1 semi-bold unselectable'><span className='text-white unselectable'>$</span>{ this.analytics?.spent_today }</p>
                            <p className='text-xs text-white regular unselectable'>Spent Today</p>
                        </div>
                    </div> 
                </div>
                <div className='total-value-spent' style={{ display: `${ this.display }` }}>
                    <div className='d-flex flex-column align-items-start p-3 lighter-bg outer-round h-100 unselectable'>
                        <div className='d-flex flex-row mb-2 align-items-center unselectable'>
                            <p className='text-sm text-white regular unselectable'>Total Value Spent</p>
                        </div>
                        <h6 className='text-lg text-white bold unselectable'>{ Number((this.total_value_spent as number).toFixed(3)) }</h6>
                        <div className='switch-toggle switch-3 switch-candy unselectable'>
                            <input id='ETH' name='selected' type='radio' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['set_value_spent', 'main', { acronym: 'ETH' }]) } }/>
                            <label className='text-xs semi-bold unselectable' htmlFor='ETH'>ETH</label>
                            <input id='SOL' name='selected' type='radio' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['set_value_spent', 'main', { acronym: 'SOL' }]) } }/>
                            <label className='text-xs semi-bold unselectable' htmlFor='SOL'>SOL</label>
                        </div>
                    </div> 
                </div>
                <div className='todays-transactions' style={{ display: `${ this.display }` }}>
                    <div className='d-flex flex-row align-items-center justify-content-between w-100 mb-4 unselectable'>
                        <h6 className='text-lg bold text-white ms-3 mb-0 unselectable'>Today's Transactions</h6>
                    </div>
                    <div className='container short-table special-scroll-bar unselectable'>
                        <table className='table table-head unselectable'>
                            <thead className='thead-dark unselectable'>
                                <tr>
                                    <th className='text-md text-white regular unselectable'>Blockchain</th>
                                    <th className='text-md text-white regular unselectable'>Transaction Hash</th>
                                    <th className='text-md text-white regular unselectable'>Date Initiated</th>
                                </tr>
                            </thead>
                            <tbody className='unselectable'>{ this.todays_transactions }</tbody>
                        </table>
                    </div>  
                </div>
            </>
        );
    };
}

export { Dashboard }