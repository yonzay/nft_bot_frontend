/* eslint-disable @typescript-eslint/no-explicit-any */
import { Contract, utils } from 'ethers';
import { JsonRpcProvider, WebSocketProvider } from '@ethersproject/providers';
import { Keypair, Connection, PublicKey } from '@solana/web3.js';
import NodeWallet from '@project-serum/anchor/dist/cjs/nodewallet';
import { Provider, Program, Idl } from '@project-serum/anchor';
import module from '../../build/Release/Module.node';
import got from 'got';

class Client {
    public static is_production = false;
    public static create_public_variable_poll_head = (task_id: number, https_url: string, chain_id: number, contract_address: string, variable_name: string, value: number, timeout: number): void => {
        let succeeded = false;
        const contract = new Contract(contract_address, [
            `function ${ variable_name }() public view returns (uint256)`
        ], new JsonRpcProvider(https_url, chain_id));
        const poll = setInterval(() => {
            contract.functions[`${ variable_name }()`].call([]).then((x: any) => {
                if (((typeof x[0] == 'number') ? x[0] : x[0].toNumber()) == value) {
                    succeeded = true;
                    module.start_polling_task(task_id);
                    clearInterval(poll);
                }
            }).catch((e: any): void => void e);
        }, 1000);
        setTimeout(() => {
            if (!succeeded) {
                clearInterval(poll);
                module.unlock_polling_task(task_id);
            }
        }, timeout);
    };
    public static create_pending_transaction_poll_head = (task_id: number, wss_url: string, chain_id: number, contract_address: string, owner_address: string, function_name: string, timeout: number): void => {
        let succeeded = false;
        let web_socket_provider: WebSocketProvider;
        try {
            web_socket_provider = new WebSocketProvider(wss_url, chain_id);
        } catch (e) {
            module.unlock_polling_task(task_id);
            return;
        }
        web_socket_provider.on('pending', txn_hash => {
            web_socket_provider.getTransaction(txn_hash).then(txn => {
                if (txn.to == contract_address) {
                    if (txn.from == owner_address) {
                        if (txn.data.substring(0, 10) == utils.id(function_name).substring(0, 10)) {
                            web_socket_provider.off('pending');
                            module.start_transaction_polling_task(task_id, txn.gasPrice, txn.gasLimit, txn.maxPriorityFeePerGas);
                            succeeded = true;
                        }
                    }
                }
            }).catch(e => void e);
        });
        setTimeout(() => {
            if (!succeeded) {
                web_socket_provider.off('pending');
                module.unlock_polling_task(task_id);
            }
        }, timeout);
    };
    public static poll_ethereum_transaction = (txn_hash: string, https_url: string, chain_id: number, discord_webhook: string): void => {
        new JsonRpcProvider(https_url, chain_id).waitForTransaction(txn_hash, 1, 600000).then((txn) => {
            got.post(discord_webhook, {
                throwHttpErrors: false,
                followRedirect: true,
                headers: {
                    'accept': 'application/json',
                    'Content-Type': 'application/json'
                },
                json: {
                    username: 'MoonDweller',
                    avatar_url: 'https://cdn.discordapp.com/attachments/599376384158072832/947648153115574302/128.png',
                    embeds: [{
                        title: txn.transactionHash,
                        url: `https://etherscan.io/tx/${ txn.transactionHash }`,
                        description: `Transaction has been successfully confirmed.`,
                        color: 8454399,
                        timestamp: new Date().toISOString(),
                        footer: {
                            text: 'MoonDweller',
                            icon_url: 'https://cdn.discordapp.com/attachments/599376384158072832/947648153115574302/128.png'
                        },
                        author: {
                            name: 'MoonDweller',
                            icon_url: 'https://cdn.discordapp.com/attachments/599376384158072832/947648153115574302/128.png'
                        },
                        fields: [
                            {
                                name: 'block_number',
                                value: txn.blockNumber,
                                inline: false
                            },
                            {
                                name: 'confirmations',
                                value: txn.confirmations,
                                inline: false
                            },
                            {
                                name: 'timestamp',
                                value: `**${ new Date() }**`,
                                inline: false
                            }
                        ]}
                    ]
                }
            }).catch(e => void e);
        }).catch(e => void e);
    };
    public static candy_machine_poll = (task_id: number, rpc_url: string, address: string, type: string): void => {
        if (type == 'CMV1') {
            const CANDY_MACHINE_PROGRAM_V1 = new PublicKey('cndyAnrLdpjq1Ssp1z8xxDsB8dxe7u4HL5Nxi2K5WXZ');
            let provider: Provider;
            try {
                provider = new Provider(new Connection(rpc_url), { publicKey: Keypair.generate().publicKey } as NodeWallet, { preflightCommitment: 'recent' });
            } catch (e) {
                module.unlock_polling_task(task_id);
                return;
            }
            const poll = setInterval(() => {
                Program.fetchIdl(CANDY_MACHINE_PROGRAM_V1, provider).then(idl => {
                    new Program(idl as Idl, CANDY_MACHINE_PROGRAM_V1, provider).account.candyMachine.fetch(address).then(candy_machine => {
                        if ((new Date().getTime() / 1000) > candy_machine.data.goLiveDate.toNumber()) {
                            module.start_polling_task(task_id);
                            clearInterval(poll);
                        }
                    }).catch(e => void e);
                }).catch(e => void e);
            }, 1000);
        } else if (type == 'CMV2') {
            const CANDY_MACHINE_PROGRAM_V2 = new PublicKey('cndy3Z4yapfJBmL3ShUp5exZKqR3z33thTzeNMm2gRZ');
            let provider: Provider;
            try {
                provider = new Provider(new Connection(rpc_url), { publicKey: Keypair.generate().publicKey } as NodeWallet, { preflightCommitment: 'recent' });
            } catch (e) {
                module.unlock_polling_task(task_id);
                return;
            }
            const poll = setInterval(() => {
                Program.fetchIdl(CANDY_MACHINE_PROGRAM_V2, provider).then(idl => {
                    new Program(idl as Idl, CANDY_MACHINE_PROGRAM_V2, provider).account.candyMachine.fetch(address).then(state => {
                        if ((new Date().getTime() / 1000) > state.data.goLiveDate.toNumber()) {
                            module.start_polling_task(task_id);
                            clearInterval(poll);
                        }
                    }).catch(e => void e);
                }).catch(e => void e);
            }, 1000);
        }
    };
}

export { Client }