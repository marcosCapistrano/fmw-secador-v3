import React, { useState } from 'react';
import csv from 'csvtojson';
import Axios from "axios";
import { Line } from 'react-chartjs-2';
import {
    Chart as ChartJS,
    CategoryScale,
    LinearScale,
    PointElement,
    LineElement,
    Title,
    Tooltip,
    Legend,
} from 'chart.js';
import { useEffect } from 'react';
import axios from 'axios';

ChartJS.register(
    CategoryScale,
    LinearScale,
    PointElement,
    LineElement,
    Title,
    Tooltip,
    Legend
);

export const options = {
    responsive: true,
    plugins: {
        legend: {
            position: 'top',
        },
        title: {
            display: true,
            text: 'Chart.js Line Chart',
        },
    },
};

function parseHistorico(json) {
    let result = {
        initDate: null,
        endDate: null,

        sublotes: []
    }

    let subloteCount = 0;
    for (let entry of json) {
        if (entry.sensor === "LOTE") {
            if (entry.value === "1") {
                result.initDate = new Date(Number(entry.date) * 1000);
            } else if (entry.value === "0") {
                result.endDate = new Date(Number(entry.date) * 1000);
            }
        } else if (entry.sensor === "DEVICE_STATE") {
            if (entry.value === "1") {
                result.sublotes.push({
                    initDate: new Date(Number(entry.date) * 1000),
                    endDate: null,

                    sensor_entr: [],
                    sensor_m1: [],
                    sensor_m2: [],
                    sensor_m3: [],
                    sensor_m4: [],
                })
            } else if (entry.value === "0") {
                result.sublotes[subloteCount].endDate = new Date(Number(entry.date) * 1000);
                subloteCount++;
            }
        } else if (entry.sensor === "SENSOR_ENTR") {
            result.sublotes[subloteCount].sensor_entr.push({
                date: new Date(Number(entry.date) * 1000),
                value: entry.value
            })
        } else if (entry.sensor === "SENSOR_M1") {
            result.sublotes[subloteCount].sensor_m1.push({
                date: new Date(Number(entry.date) * 1000),
                value: entry.value
            })
        } else if (entry.sensor === "SENSOR_M2") {
            result.sublotes[subloteCount].sensor_m2.push({
                date: new Date(Number(entry.date) * 1000),
                value: entry.value
            })
        } else if (entry.sensor === "SENSOR_M3") {
            result.sublotes[subloteCount].sensor_m3.push({
                date: new Date(Number(entry.date) * 1000),
                value: entry.value
            })
        } else if (entry.sensor === "SENSOR_M4") {
            result.sublotes[subloteCount].sensor_m4.push({
                date: new Date(Number(entry.date) * 1000),
                value: entry.value
            })
        }
    }

    return result;
}

let rawData = `1664313661,LOTE,1
1664313661,DEVICE_STATE,1
1664313661,SENSOR_ENTR,0
1664313661,SENSOR_M1,0
1664313661,SENSOR_M2,0
1664313661,SENSOR_M3,0
1664313661,SENSOR_M4,0
1664313662,SENSOR_ENTR,22`;

const labels = ['January', 'February', "march"];

export default function LoteDetails() {
    const [initDate, setInitDate] = useState(null);
    const [endDate, setEndDate] = useState(null);
    const [data, setData] = useState(null);

    useEffect(() => {
        async function fetchData() {
            // const response = await MyAPI.getData(someId);
            rawData = "date, sensor, value\n" + rawData;
            csv().fromString(rawData.toString()).then(json => {
                let result = parseHistorico(json);

                setInitDate(result.initDate);
                setEndDate(result.endDate);

                console.log();

                setData({
                    labels,
                    datasets: [
                        {
                            label: 'Dataset 1',
                            data: result.sublotes[0].sensor_entr.map(data => Number(data.value)),
                            borderColor: 'rgb(255, 99, 132)',
                            backgroundColor: 'rgba(255, 99, 132, 0.5)',
                        },
                    ],
                })
            })
        }
        fetchData();

        // try {
        // const response = await axios.get('/lote/1');
        // rawData = response.data.replace("\n", ",");
        // rawData = rawData.replace("\n", ",");
        // console.log(rawData);
        // } catch (error) {
        //     console.error(error);
        // }




        // }, [someId]); // Or [] if effect doesn't need props or state
    }, []);

    if (!data) {
        return <pre>Loading...</pre>
    } else {
        console.log(data)
    }

    return (
        <>
            <h1 className="text-center text-3xl pb-8">Lote 1</h1>
            <Line options={options} data={data} />;
            {/* <Line
                options={options}
                data={data}
            /> */}
        </>
    )
}