/* eslint-disable @typescript-eslint/no-explicit-any */
import React from 'react';
import Particle from 'react-tsparticles';
import particle_config from './components/particles_config.json';
import { RecursivePartial, IOptions } from 'tsparticles';
import { TitleBar } from './components/title_bar';
import { SideBar } from './components/side_bar';
import { Dashboard } from './components/dashboard';
import { ContractTasks } from './components/contract_tasks';
import { SniperTasks } from './components/sniper_tasks';
import { WalletGroups } from './components/wallet_groups';
import { TransactionHistory } from './components/transaction_history';

class Main {
    public static Make: React.FC = () => {
        return (
            <>
                <TitleBar.Make id='main' mode='main' title='MoonDweller'/>
                <div className='base'>
                    <Particle className='main-particles' options={ particle_config as RecursivePartial<IOptions> }/>
                    <SideBar.Make/>
                    <Dashboard.Make/>
                    <ContractTasks.Make/>
                    <SniperTasks.Make/>
                    <WalletGroups.Make/>
                    <TransactionHistory.Make/>
                </div>
            </>
        );
    };
}

(window as any).TitleBar = TitleBar;
(window as any).SideBar = SideBar;
(window as any).Dashboard = Dashboard;
(window as any).ContractTasks = ContractTasks;
(window as any).SniperTasks = SniperTasks;
(window as any).WalletGroups = WalletGroups;
(window as any).TransactionHistory = TransactionHistory;

export { Main }