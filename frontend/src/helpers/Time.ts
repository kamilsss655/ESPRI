import { date } from 'quasar';

export const useFormattedTimeStamp = (): string => {
    return date.formatDate(Date.now(), 'YYYYMMDD_HHmmss');
};