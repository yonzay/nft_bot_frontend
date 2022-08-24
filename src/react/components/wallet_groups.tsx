/* eslint-disable @typescript-eslint/no-explicit-any */
import React, { useState } from 'react';

type show = 'none' | 'block'

interface WalletGroupEntry {
    name: string;
    blockchain: string;
    private_keys: number;
    value: number;
    acronym: string;
    market_value: number;
}

class WalletGroups {
    private static display: show | undefined;
    public static update_display: React.Dispatch<React.SetStateAction<show | undefined>>;
    public static wallet_groups: React.DetailedHTMLProps<React.HTMLAttributes<HTMLDivElement>, HTMLDivElement>[] | undefined;
    private static update_wallet_groups: React.Dispatch<React.SetStateAction<React.DetailedHTMLProps<React.HTMLAttributes<HTMLDivElement>, HTMLDivElement>[] | undefined>>;
    public static tsx_wrap_wallet_groups = (array: WalletGroupEntry[]): void => {
        const react_array: React.DetailedHTMLProps<React.HTMLAttributes<HTMLDivElement>, HTMLDivElement>[] = [];
        array.forEach((wallet_group) => {
            react_array.push(
                <div className='wallet d-flex flex-column align-items-start lighter-bg has-event'>
                    <div className='text-sm text-white semi-bold mb-2 wallet-title'>{ wallet_group.name }</div>
                    <div className='text-sm text-white semi-bold cust-mb'>{ wallet_group.blockchain }</div>
                    <div className='d-flex flex-row align-items-center justify-content-center'>
                        <span className='text-white text-sm cust-mb me-1'>{ wallet_group.private_keys }</span><div className='text-sm text-white cust-mb semi-bold'>Private Keys</div>
                    </div>
                    <div className='d-flex flex-row align-items-center justify-content-center'>
                        <span className='text-white text-sm me-1'>{ Number(wallet_group.value.toFixed(3)) }</span>
                        <div className='text-sm text-purple cust-mb semi-bold'>{ wallet_group.acronym }</div>
                    </div>
                    <div className='text-sm text-purple cust-mb'><span className='text-white semi-bold'>$</span><span className='text-white me-1'>{ wallet_group.market_value }</span><span className='text-purple semi-bold'>USD</span></div>
                    <div className='d-flex flex-row align-items-center justify-content-between w-100 mt-2'>
                        <div className='transparent-btn text-sm text-white d-flex flex-row align-items-cneter edit-btn' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['open', 'edit_wallet_group', { wallet_group_name: wallet_group.name }]) } }>Edit</div>
                        <div className='transparent-btn text-sm text-white d-flex flex-row align-items-cneter delete-btn' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['open', 'confirmation', { confirm_type: 'wallet_group', wallet_group_name: wallet_group.name, wallet_group_private_keys: wallet_group.private_keys, wallet_group_value: wallet_group.value, value_acronym: wallet_group.acronym, market_value: wallet_group.market_value }]) } }>Delete</div>
                    </div>
                </div>
            );
        });
        this.update_wallet_groups(react_array);
    };
    public static Make: React.FC = () => {
        [this.display, this.update_display] = useState<show | undefined>('none');
        [this.wallet_groups, this.update_wallet_groups] = useState<React.DetailedHTMLProps<React.HTMLAttributes<HTMLDivElement>, HTMLDivElement>[] | undefined>();
        return (
            <div className='other-pages' style={{ display: `${ this.display }` }}>
                <div className='d-flex flex-row align-items-center justify-content-between w-100 mb-4 unselectable'>
                    <div className='d-flex flex-row align-items-center justify-content-between w-100 unselectable'>
                        <div className='placement text-lg bold text-white ms-3 mb-0 unselectable'>Manage Wallet Groups</div>
                        <div className='d-flex flex-row unselectable'>
                            <div className='transparent-btn text-sm text-white d-flex flex-row align-items-center me-3 unselectable' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['refresh_balances', 'main', null]) } }>Refresh Balances</div>
                            <div className='transparent-btn text-sm text-white d-flex flex-row align-items-center me-3 unselectable' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['open', 'generate_wallet', null]) } }>Generate Wallet</div>
                            <div className='transparent-btn text-sm text-white d-flex flex-row align-items-center me-3 unselectable' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['open', 'create_wallet_group', null]) } }>Create Wallet Group</div>
                        </div>
                    </div>
                </div>
                <div className='container wallet-container long-table wallet-groups-scroll-bar unselectable'>{ this.wallet_groups }</div>
            </div>
        );
    };
}

export { WalletGroups }