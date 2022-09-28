import React, { useState } from 'react';
import csv from 'csvtojson';
import Axios from "axios";
import { useEffect } from 'react';
import axios from 'axios';
import LoteAreaChart from '../components/LoteAreaChart';


let rawData = `1664313661,LOTE,1
1664313661,DEVICE_STATE,1
1664313661,SENSOR_ENTR,0
1664313661,SENSOR_M1,0
1664313661,SENSOR_M2,0
1664313661,SENSOR_M3,0
1664313661,SENSOR_M4,0
1664313661,SENSOR_ENTR, 22
1664313661,SENSOR_M1,23
1664313661,SENSOR_M2,32
1664313661,SENSOR_M3,25
1664313661,SENSOR_M4,21
1664313671,SENSOR_ENTR, 24
1664313681,SENSOR_M1,23
1664313691,SENSOR_M2,0
1664313701,SENSOR_M3,25
1664313711,SENSOR_M4,12
1664313721,SENSOR_ENTR, 22
1664313731,SENSOR_M1,23
1664313751,SENSOR_M2,0
1664318061,SENSOR_M3,25
1664318361,SENSOR_M4,0
1664318361,SENSOR_ENTR, 22
1664318361,SENSOR_M1,23
1664319261,SENSOR_M2,21
1664319761,SENSOR_M3,25
1664320661,SENSOR_M4,21
1664333662,SENSOR_ENTR,22
1664343661,DEVICE_STATE,0
1664353661,DEVICE_STATE,1
1664363681,SENSOR_M1,23
1664373691,SENSOR_M2,0
1664383701,SENSOR_M3,25
1664393711,SENSOR_M4,12
1664403721,SENSOR_ENTR, 22
1664413731,SENSOR_M1,23
1664423751,SENSOR_M2,0
1664438061,SENSOR_M3,25
1664448361,SENSOR_M4,0
1664458361,SENSOR_ENTR, 22
1664468361,SENSOR_M1,23
1664479261,SENSOR_M2,21
1664489761,SENSOR_M3,25
1664490661,SENSOR_M4,21
1664503662,SENSOR_ENTR,22
`;

export default function LoteDetails() {
    const [loading, setLoading] = useState(true);
    const [initDate, setInitDate] = useState(null);
    const [endDate, setEndDate] = useState(null);
    const [sublotes, setSublotes] = useState([]);

    useEffect(() => {
        async function fetchData() {
            // const response = await MyAPI.getData(someId);
            rawData = "date, sensor, value\n" + rawData;
            csv().fromString(rawData.toString()).then(json => {
                let result = parseHistorico(json);

                setInitDate(result.initDate);
                setEndDate(result.endDate);
                setSublotes(result.sublotes);
                setLoading(false);
            });
        }

        fetchData();
    }, []);

    if (loading) {
        return <pre>Loading...</pre>
    }

    return (
        <>
            <h1 className="text-center text-3xl pb-8">Lote 1</h1>
            {sublotes.map(sublote => (
                <div className='items-center'>
                    <LoteAreaChart sublote={sublote} />
                </div>
            ))}
        </>
    )
}

function parseHistorico(json) {
    let result = {
        initDate: null,
        endDate: null,

        sublotes: []
    }

    let validLote = false;
    let subloteCount = 0;
    for (let i = 0; i < json.length; i++) {
        let entry = json[i];
        let date = Number(entry.date);
        let value = entry.value;

        if (entry.sensor === "LOTE") {
            if (value === "1") {
                result.initDate = date;
                validLote = true;
            } else if (entry.value === "0") {
                result.endDate = date;
                validLote = false;
            }
        }

        if (validLote) {
            if (entry.sensor === "DEVICE_STATE") {
                if (value === "1") {
                    result.sublotes.push({
                        initDate: date,
                        endDate: null,

                        labels: [],
                        sensor_entr: [],
                        sensor_m1: [],
                        sensor_m2: [],
                        sensor_m3: [],
                        sensor_m4: [],
                    })
                } else if (value === "0") {
                    result.sublotes[subloteCount].endDate = date;
                    subloteCount++;
                }

            } else if (entry.sensor === "SENSOR_ENTR") {
                result.sublotes[subloteCount].sensor_entr.push({
                    date: date,
                    value: value
                })

                if (result.sublotes[subloteCount].labels.includes(date) === false)
                    result.sublotes[subloteCount].labels.push(date);
            } else if (entry.sensor === "SENSOR_M1") {
                result.sublotes[subloteCount].sensor_m1.push({
                    date: date,
                    value: value
                })

                if (result.sublotes[subloteCount].labels.includes(date) === false)
                    result.sublotes[subloteCount].labels.push(date);
            } else if (entry.sensor === "SENSOR_M2") {
                result.sublotes[subloteCount].sensor_m2.push({
                    date: date,
                    value: value
                })

                if (result.sublotes[subloteCount].labels.includes(date) === false)
                    result.sublotes[subloteCount].labels.push(date);
            } else if (entry.sensor === "SENSOR_M3") {
                result.sublotes[subloteCount].sensor_m3.push({
                    date: date,
                    value: value
                })

                if (result.sublotes[subloteCount].labels.includes(date) === false)
                    result.sublotes[subloteCount].labels.push(date);
            } else if (entry.sensor === "SENSOR_M4") {
                result.sublotes[subloteCount].sensor_m4.push({
                    date: date,
                    value: value
                })

                if (result.sublotes[subloteCount].labels.includes(date) === false)
                    result.sublotes[subloteCount].labels.push(date);
            }
        }
    }

    return result;
}