/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Modal } from '../modal';
import '../../style/style.css';
import '../../style/modal_style.css';

class Settings_Index {
	public static settings = (input: string): void => {
		(window as any).electron.ipcRenderer.message('perform', ['submit_action', 'settings', input]);
	};
	public static create = (settings_json: string): void => {
		const Element: Modal = new Modal(undefined, 'settings', 'Settings', 'modal', 'block', 'json', 'ace-modal', false, JSON.stringify(JSON.parse(settings_json), null, '\t'), this.settings);
		ReactDOM.render(<Element.Make/>,
		document.getElementById('root'));
	};
}

(window as any).Settings_Index = Settings_Index;