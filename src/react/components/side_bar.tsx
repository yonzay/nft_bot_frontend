/* eslint-disable @typescript-eslint/no-explicit-any */
import React, { ReactElement, useState } from 'react';
import { Dashboard } from './dashboard';
import { ContractTasks } from './contract_tasks';
import { SniperTasks } from './sniper_tasks';
import { WalletGroups } from './wallet_groups';
import { TransactionHistory } from './transaction_history';
import dashboard_img from '../assets/dashboard.png';
import contract_tasks_img from '../assets/contract_tasks.png';
import sniper_tasks_img from '../assets/sniper_tasks.png';
import wallet_groups_img from '../assets/wallet_groups.png';
import transactions_img from '../assets/transactions.png';

type frame = 'dashboard' | 'contract_tasks' | 'sniper_tasks' | 'wallet_groups' | 'transactions'

interface InstanceSummary {
    ethereum_spent: number;
    solana_spent: number;
    current_gas: number;
    txns_attempted: number;
    total_contract_tasks: number;
    contract_tasks_active: number;
    total_sniper_tasks: number;
    sniper_tasks_active: number;
}

interface SubscriptionInfo {
    renewal_date: number;
    last_login: number;
    wallet_address: string;
}

class SideBar {
    public static instance_summary: InstanceSummary | undefined;
    public static update_instance_summary: React.Dispatch<React.SetStateAction<InstanceSummary | undefined>>;
    public static subscription_info: SubscriptionInfo | undefined;
    public static update_subscription_info: React.Dispatch<React.SetStateAction<SubscriptionInfo | undefined>>;
    private static current_frame: frame;
    private static set_frame = (target_state: frame): void => {
        switch (target_state) {
            case 'dashboard':
                if (this.current_frame !== target_state) {
                    this.current_frame = target_state;
                    Dashboard.update_display('block');
                    ContractTasks.update_display('none');
                    SniperTasks.update_display('none');
                    WalletGroups.update_display('none');
                    TransactionHistory.update_display('none');
                }
                return;
            case 'contract_tasks':
                if (this.current_frame !== target_state) {
                    this.current_frame = target_state;
                    Dashboard.update_display('none');
                    ContractTasks.update_display('block');
                    SniperTasks.update_display('none');
                    WalletGroups.update_display('none');
                    TransactionHistory.update_display('none');
                }
                return;
            case 'sniper_tasks':
                if (this.current_frame !== target_state) {
                    this.current_frame = target_state;
                    Dashboard.update_display('none');
                    ContractTasks.update_display('none');
                    SniperTasks.update_display('block');
                    WalletGroups.update_display('none');
                    TransactionHistory.update_display('none');
                }
                return;
            case 'wallet_groups':
                if (this.current_frame !== target_state) {
                    this.current_frame = target_state;
                    Dashboard.update_display('none');
                    ContractTasks.update_display('none');
                    SniperTasks.update_display('none');
                    WalletGroups.update_display('block');
                    TransactionHistory.update_display('none');
                }
                return;
            case 'transactions':
                 if (this.current_frame !== target_state) {
                    this.current_frame = target_state;
                    Dashboard.update_display('none');
                    ContractTasks.update_display('none');
                    SniperTasks.update_display('none');
                    WalletGroups.update_display('none');
                    TransactionHistory.update_display('block');
                }
                return;
        }
    };
    public static Make: React.FC = (): ReactElement => {
        [this.instance_summary, this.update_instance_summary] = useState<InstanceSummary>();
        [this.subscription_info, this.update_subscription_info] = useState<SubscriptionInfo>();
        return (
            <div className='side-bar'>
                <div className='side-nav d-flex flex-column justify-content-center align-items-center p-3 pt-3 unselectable'>
                    <div className='nav-item d-flex flex-row align-items-center mb-3 unselectable' onClick={ () => this.set_frame('dashboard') }>
                        <img className='unselectable' src={ dashboard_img } alt=''/>
                        <p className='text-white text-sm light mb-0 ms-3 unselectable'>Dashboard</p>
                    </div>
                    <div className='nav-item d-flex flex-row align-items-center mb-3 unselectable' onClick={ () => this.set_frame('contract_tasks') }>
                        <img className='unselectable' src={ contract_tasks_img } alt=''/>
                        <p className='text-white text-sm light mb-0 ms-3 unselectable'>Contract Tasks</p>
                    </div>
                    <div className='nav-item d-flex flex-row align-items-center mb-3 unselectable' onClick={ () => this.set_frame('sniper_tasks') }>
                        <img className='unselectable' src={ sniper_tasks_img } alt=''/>
                        <p className='text-white text-sm light mb-0 ms-3 unselectable'>Sniper Tasks</p>
                    </div>
                    <div className='nav-item d-flex flex-row align-items-center mb-3 unselectable' onClick={ () => this.set_frame('wallet_groups') }>
                        <img className='unselectable' src={ wallet_groups_img } alt=''/>
                        <p className='text-white text-sm light mb-0 ms-3 unselectable'>Wallet Groups</p>
                    </div>
                    <div className='nav-item d-flex flex-row align-items-center unselectable' onClick={ () => this.set_frame('transactions') }>
                        <img className='unselectable' src={ transactions_img } alt=''/>
                        <p className='text-white text-sm light mb-0 ms-3 unselectable'>Transactions</p>
                    </div>
                    <div className='stats w-100 inner-round p-3 mb-3 light-border lighter-bg mt-3 unselectable'>
                        <div className='d-flex flex-row align-items-start justify-content-start w-100 mb-2 unselectable'>
                            <p className='text-sm text-white regular me-1 unselectable'>Spent This Session:</p>
                        </div>
                        <div className='d-flex flex-row align-items-start justify-content-start w-100 mb-2 unselectable'>
                            <p className='text-sm text-white regular me-1 unselectable'>
                                { Number(this.instance_summary?.ethereum_spent.toFixed(3)) } <span className='text-purple semi-bold unselectable'>ETH</span>
                                <br/>
                                { Number(this.instance_summary?.solana_spent.toFixed(3)) } <span className='text-purple semi-bold unselectable'>SOL</span>
                            </p>
                        </div>
                        <div className='d-flex flex-row align-items-start justify-content-start w-100 mb-2 unselectable'>
                            <p className='text-sm text-white regular me-1 unselectable'>Current Gas: { this.instance_summary?.current_gas } <span className='text-purple semi-bold unselectable'>GWEI</span></p>
                        </div>
                        <div className='d-flex flex-row align-items-center justify-content-start w-100 mb-2 unselectable'>
                            <p className='text-sm text-white regular me-1 unselectable'>Txns Attempted: { this.instance_summary?.txns_attempted }</p>
                        </div>
                        <div className='d-flex flex-row align-items-start justify-content-start w-100 mb-2 unselectable'>
                            <p className='text-sm text-white regular me-1 unselectable'>Total Contract Tasks: { this.instance_summary?.total_contract_tasks }</p>
                        </div>
                        <div className='d-flex flex-row align-items-start justify-content-start w-100 mb-2 unselectable'>
                            <p className='text-sm text-white regular me-1 unselectable'>Contract Tasks Active: { this.instance_summary?.contract_tasks_active }</p>
                        </div>
                        <div className='d-flex flex-row align-items-start justify-content-start w-100 mb-2 unselectable'>
                            <p className='text-sm text-white regular me-1 unselectable'>Total Sniper Tasks: { this.instance_summary?.total_sniper_tasks }</p>
                        </div>
                        <div className='d-flex flex-row align-items-start justify-content-start w-100 unselectable'>
                            <p className='text-sm text-white regular me-1 unselectable'>Sniper Tasks Active: { this.instance_summary?.sniper_tasks_active }</p>
                        </div>
                    </div>
                    <div className='hyper-info p-3 w-100 inner-round light-border lighter-bg unselectable'>
                        <div className='d-flex flex-row align-items-center justify-content-start w-100 mb-2 unselectable'>
                            <p className='text-sm text-white regular me-1 unselectable'>Renewal Date: { new Date(this.subscription_info?.renewal_date ?? 0).toISOString().split('T')[0] }</p>
                        </div>
                        <div className='d-flex flex-row align-items-center justify-content-start w-100 mb-2 unselectable'>
                            <p className='text-sm text-white regular me-1 unselectable'>Last Login: { new Date(this.subscription_info?.last_login ?? 0).toISOString().split('T')[0] }</p>
                        </div>
                        <div className='d-flex flex-row align-items-start justify-content-start w-100 unselectable'>
                            <p className='text-sm text-white regular me-1 w-100 wrap-anywhere unselectable'>Wallet Address: <span className='add-underline unselectable' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['href', 'main', { wallet: this.subscription_info?.wallet_address } ]) } }>{ this.subscription_info?.wallet_address }</span></p>
                        </div>
                    </div>
                </div>
            </div>
        );
    };
}

export { SideBar }