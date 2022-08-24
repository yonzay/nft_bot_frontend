/* eslint-disable @typescript-eslint/no-explicit-any */
import { contextBridge, ipcRenderer } from 'electron';

type command = 'open' | 'minimize' | 'maximize' | 'close' | 'href' | 'set_value_spent' |'submit_action' | 'play' | 'pause' | 'refresh_balances' | 'confirm';
type bridge_windows = 'chronicle' | 'authentication' | 'key_derivation' | 'decrypt' | 'settings' | 'main' | 'create_contract_task' | 'view_contract_task' | 
'create_sniper_task' | 'view_sniper_task' | 'generate_wallet' | 'create_wallet_group' | 'edit_wallet_group' | 'confirmation';
type peform_argument = [command, bridge_windows, any];

contextBridge.exposeInMainWorld('electron', {
	ipcRenderer: {
		message: (channel: string, args: peform_argument) => {
			ipcRenderer.send(channel, args);
		}
	}
});

export { peform_argument }