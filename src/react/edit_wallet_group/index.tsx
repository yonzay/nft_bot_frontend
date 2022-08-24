/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Modal } from '../modal';
import '../../style/style.css';
import '../../style/modal_style.css';

class Edit_Wallet_Group_Index {
	public static wallet_group_name: string;
	public static edit_wallet_group = (input: string): void => {
		(window as any).electron.ipcRenderer.message('perform', ['submit_action', 'edit_wallet_group', [this.wallet_group_name, input]]);
	};
	public static create = (wallet_group_name: string, private_keys_json: string): void => {
		this.wallet_group_name = wallet_group_name;
		const Element: Modal = new Modal(undefined, `edit_wallet_group_${ this.wallet_group_name }`, `Edit Wallet Group - ${ this.wallet_group_name }`, 'modal', 'block', 'json', 'ace-modal', false, JSON.stringify(JSON.parse(private_keys_json), null, '\t'), this.edit_wallet_group);
		ReactDOM.render(<Element.Make/>,
		document.getElementById('root'));
	};
}

(window as any).Edit_Wallet_Group_Index = Edit_Wallet_Group_Index;